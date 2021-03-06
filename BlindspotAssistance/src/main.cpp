// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
/**
* \brief The entry point for the Inference Engine blindspot_assistance application
* \file BlindspotAssistance/main.cpp
* \example BlindspotAssistance/main.cpp
*/
#include <iostream>
#include <vector>
#include <utility>

#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <queue>
#include <chrono>
#include <sstream>
#include <memory>
#include <string>
#include <fstream>

#ifdef USE_TBB
#include <tbb/parallel_for.h>
#endif

#include <opencv2/opencv.hpp>
#include "drawer.hpp"

#include <monitors/presenter.h>
#include <samples/slog.hpp>
#include <samples/args_helper.hpp>

#include "input.hpp"
#include "multichannel_params.hpp"
#include "blindspot_params.hpp"
#include "output.hpp"
#include "threading.hpp"
#include "graph.hpp"

#include "alert_publisher.hpp"
#include "vehicle_status.hpp"

namespace
{

    /**
* \brief This function show a help message
*/
    void showUsage()
    {
        std::cout << std::endl;
        std::cout << "blindspot_assistance [OPTION]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << std::endl;
        std::cout << "    -h                           " << help_message << std::endl;
        std::cout << "    -m \"<path>\"                  " << model_path_message << std::endl;
        std::cout << "      -l \"<absolute_path>\"       " << custom_cpu_library_message << std::endl;
        std::cout << "          Or" << std::endl;
        std::cout << "      -c \"<absolute_path>\"       " << custom_cldnn_message << std::endl;
        std::cout << "    -d \"<device>\"                " << target_device_message << std::endl;
        std::cout << "    -nc                          " << num_cameras << std::endl;
        std::cout << "    -bs                          " << batch_size << std::endl;
        std::cout << "    -nireq                       " << num_infer_requests << std::endl;
        std::cout << "    -n_iqs                       " << input_queue_size << std::endl;
        std::cout << "    -fps_sp                      " << fps_sampling_period << std::endl;
        std::cout << "    -n_sp                        " << num_sampling_periods << std::endl;
        std::cout << "    -pc                          " << performance_counter_message << std::endl;
        std::cout << "    -t                           " << thresh_output_message << std::endl;
        std::cout << "    -no_show                     " << no_show_processed_video << std::endl;
        std::cout << "    -no_show_d                   " << no_show_detection << std::endl;
        std::cout << "    -show_stats                  " << show_statistics << std::endl;
        std::cout << "    -duplicate_num               " << duplication_channel_number << std::endl;
        std::cout << "    -real_input_fps              " << real_input_fps << std::endl;
        std::cout << "    -i                           " << input_video << std::endl;
        std::cout << "    -loop_video                  " << loop_video_output_message << std::endl;
        std::cout << "    -u                           " << utilization_monitors_message << std::endl;
        std::cout << "    -calibration                 " << calibration_message << std::endl;
        std::cout << "    -show_calibration            " << show_calibration_message << std::endl;
        std::cout << "    -alerts                      " << alerts_message << std::endl;
        std::cout << "    -dm                          " << driver_mode << std::endl;
        std::cout << "    -msg_bus                     " << eis_msg_bus << std::endl;
    }

    bool ParseAndCheckCommandLine(int argc, char *argv[])
    {
        // ---------------------------Parsing and validation of input args--------------------------------------
        gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
        if (FLAGS_h)
        {
            showUsage();
            showAvailableDevices();
            return false;
        }
        slog::info << "Parsing input parameters" << slog::endl;

        if (FLAGS_m.empty())
        {
            throw std::logic_error("Parameter -m is not set");
        }
        if (FLAGS_nc == 0 && FLAGS_i.empty())
        {
            throw std::logic_error("Please specify at least one video source(web cam or video file)");
        }
        slog::info << "\tDetection model:           " << FLAGS_m << slog::endl;
        slog::info << "\tDetection threshold:       " << FLAGS_t << slog::endl;
        slog::info << "\tUtilizing device:          " << FLAGS_d << slog::endl;
        if (!FLAGS_l.empty())
        {
            slog::info << "\tCPU extension library:     " << FLAGS_l << slog::endl;
        }
        if (!FLAGS_c.empty())
        {
            slog::info << "\tCLDNN custom kernels map:  " << FLAGS_c << slog::endl;
        }
        slog::info << "\tBatch size:                " << FLAGS_bs << slog::endl;
        slog::info << "\tNumber of infer requests:  " << FLAGS_nireq << slog::endl;
        slog::info << "\tNumber of input web cams:  " << FLAGS_nc << slog::endl;

        return true;
    }

    struct Detection
    {
        cv::Rect2f rect;
        int label;
        float confidence;
        Detection(cv::Rect2f r, int l, float c) : rect(r), label(l), confidence(c) {}
    };

    // Globals
    const size_t DISP_WIDTH = 1280;
    const size_t DISP_HEIGHT = 720;
    const size_t MAX_INPUTS = 4;
    bool firstTime = true;
    cv::Rect2d roi[MAX_INPUTS];
    int camDetections[MAX_INPUTS];
    Publisher* g_publisher = NULL;
    MessageQueue* g_input_queue = NULL;

    void drawDetections(cv::Mat &img, const std::vector<Detection> &detections)
    {
        cv::Scalar color;
        for (const Detection &f : detections)
        {
            if (f.label == 1)
            {
                color = cv::Scalar(255, 0, 0);
            }
            else if (f.label == 2)
            {
                color = cv::Scalar(0, 255, 0);
            }
            else
            {
                color = cv::Scalar(0, 0, 255);
            }
            cv::Rect ri(static_cast<int>(f.rect.x * img.cols), static_cast<int>(f.rect.y * img.rows),
                        static_cast<int>(f.rect.width * img.cols), static_cast<int>(f.rect.height * img.rows));
            cv::rectangle(img, ri, color, 2);
        }
    }

    void alertHandler(size_t i, const Detection &f, VehicleStatus *vehicle){

        vehicle->find_mode();
        time_t now = time(0);
        tm *ltm = localtime(&now);
        char date[11], time[10], char_array[50]; 
        int payload_size, message_size;
        ExampleMessage* wrap;

        snprintf ((char *) date, 11, "%.2d/%.2d/%.4d", ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
        snprintf ((char *) time, 10, "%.2d:%.2d:%.2d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

        std::string payload = std::string(date)+","+std::string(time)+","+std::to_string(i)+","+std::to_string(f.label)+","+std::to_string(f.confidence)+
                                ","+vehicle->get_mode_to_string();

        memcpy(char_array, payload.c_str(), 50);
        wrap = new ExampleMessage(char_array);
        std::cout << "Enquing message to send: " << char_array << std::endl;
        g_input_queue->push(wrap);
    }

    int areaDetectionCount(cv::Mat &img, const std::vector<Detection> &detections, size_t i, cv::Rect2d roi, VehicleStatus *vehicle)
    {
        int count = 0;

        for (const Detection &f : detections)
        {
            // Central Points of the Detections
            float x = (f.rect.x + f.rect.width / 2) * (float)(img.cols);
            float y = (f.rect.y + f.rect.height / 2) * (float)(img.rows);

            // Check if point is inside a ROI
            if (x > roi.x && x < roi.x + roi.height)
            {
                if (y > roi.y && y < roi.y + roi.height)
                {
                    count += 1;

                    // Send alert if enable
                    if (strlen(FLAGS_msg_bus.c_str()) > 0 && FLAGS_alerts){
                        alertHandler(i+1, f, vehicle);
                    } 
                }
            }
        }
        return count;
    }

    void readArea()
    {
        // Read area points from CSV
        std::ifstream pointsFile("../../../utils/points.ini");
        std::string line;
        int i = 0;
        int points[MAX_INPUTS][4];
        if (!pointsFile.is_open()) // Check if file is really open
        {
            std::cout << "Unable to upload init Area Configuration" << std::endl;
        }
        else
        {
            std::cout << "Reading Area Configuration" << std::endl;
            while (getline(pointsFile, line))
            {
                std::stringstream sst(line);
                sst >> points[i][0] >> points[i][1] >> points[i][2] >> points[i][3];
                std::cout << "Cam Area " << i + 1 << ": " << points[i][0] << ',' << points[i][1] << ',' << points[i][2] << ',' << points[i][3] << std::endl;
                i++;
            }
        }
    }

    void saveArea(cv::Rect2d roi)
    {
        // Code to save area points in CSV.
    }

    cv::Rect2d areaDetection(cv::Mat windowImage, int i, cv::Point params, cv::Size frameSize)
    {
        cv::Rect2d roiCam;
        cv::Rect crop(params.x, params.y, frameSize.width, frameSize.height);
        std::string windowName = "Select Detection Area. Cam: " + std::to_string(i + 1);
        roiCam = cv::selectROI(windowName, windowImage(crop), false);
        cv::destroyWindow(windowName);
        return roiCam;
    }

    void drawAreaDetection(cv::Mat &img, cv::Rect2d roi, cv::Point params)
    {
        roi.x += params.x;
        roi.y += params.y;
        cv::rectangle(img, roi, cv::Scalar(0, 0, 0), 1);
    }

    struct DisplayParams
    {
        std::string name;
        cv::Size windowSize;
        cv::Size frameSize;
        size_t count;
        cv::Point points[MAX_INPUTS];
    };

    DisplayParams prepareDisplayParams(size_t count)
    {
        DisplayParams params;
        params.count = count;
        params.windowSize = cv::Size(DISP_WIDTH, DISP_HEIGHT);

        size_t gridCount = static_cast<size_t>(ceil(sqrt(count)));
        size_t gridStepX = static_cast<size_t>(DISP_WIDTH / gridCount);
        size_t gridStepY = static_cast<size_t>(DISP_HEIGHT / gridCount);
        params.frameSize = cv::Size(gridStepX, gridStepY);

        for (size_t i = 0; i < count; i++)
        {
            cv::Point p;
            p.x = gridStepX * (i / gridCount);
            p.y = gridStepY * (i % gridCount);
            params.points[i] = p;
        }
        return params;
    }

    void displayNSources(const std::vector<std::shared_ptr<VideoFrame>> &data,
                         float time, const std::string &stats,
                         DisplayParams params, Presenter &presenter,
                         VehicleStatus *vehicle)
    {
        cv::Mat windowImage = cv::Mat::zeros(params.windowSize, CV_8UC3);
        auto loopBody = [&](size_t i) {
            auto &elem = data[i];
            if (!elem->frame.empty())
            {
                cv::Rect rectFrame = cv::Rect(params.points[i], params.frameSize);
                cv::Mat windowPart = windowImage(rectFrame);
                cv::resize(elem->frame, windowPart, params.frameSize);
                if (!FLAGS_no_show_d)
                {
                    drawDetections(windowPart, elem->detections.get<std::vector<Detection>>());
                }
                camDetections[i] = areaDetectionCount(windowPart, elem->detections.get<std::vector<Detection>>(), i, roi[i], vehicle);
            }
        };

        auto drawStats = [&]() {
            if (FLAGS_show_stats && !stats.empty())
            {
                static const cv::Point posPoint = cv::Point(20, 20);
                auto pos = posPoint + cv::Point(0, 35);
                size_t currPos = 0;
                while (true)
                {
                    auto newPos = stats.find('\n', currPos);
                    cv::putText(windowImage, stats.substr(currPos, newPos - currPos), pos, cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
                    cv::putText(windowImage, stats.substr(currPos, newPos - currPos), pos, cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                    if (newPos == std::string::npos)
                    {
                        break;
                    }
                    pos += cv::Point(0, 20);
                    currPos = newPos + 1;
                }
            }
        };

//  #ifdef USE_TBB
#if 0 // disable multithreaded rendering for now
    run_in_arena([&](){
        tbb::parallel_for<size_t>(0, data.size(), [&](size_t i) {
            loopBody(i);
        });
    });
#else
        for (size_t i = 0; i < data.size(); ++i)
        {
            loopBody(i);
        }
#endif
        presenter.drawGraphs(windowImage);

        // Select Area Detection
        if (FLAGS_calibration && firstTime)
        {
            std::cout << "Start area detection configuration" << std::endl;
            for (int i = 0; i < MAX_INPUTS; i++)
            {
                std::cout << "Selec Area Detection. Cam: " << std::to_string(i + 1) << std::endl;
                roi[i] = areaDetection(windowImage, i, params.points[i], params.frameSize);
            }
            /* saveArea(roi); */
            firstTime = false;
        }

        // Draw Area Detection
        if (FLAGS_show_calibration)
        {
            for (int i = 0; i < MAX_INPUTS; i++)
            {
                drawAreaDetection(windowImage, roi[i], params.points[i]);
            }
        }

        drawStats();

        if (FLAGS_show_stats)
        {
            char str[256];
            snprintf(str, sizeof(str), "%5.2f fps", static_cast<double>(1000.0f / time));
            cv::putText(windowImage, str, cv::Point(15, 30), cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 5);
            cv::putText(windowImage, str, cv::Point(15, 30), cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
        }

        cv::imshow(params.name, windowImage);
    }

} // namespace

int main(int argc, char *argv[])
{
    try
    {
        VehicleStatus vehicle;
    
#if USE_TBB
        TbbArenaWrapper arena;
#endif
        slog::info << "InferenceEngine: " << InferenceEngine::GetInferenceEngineVersion() << slog::endl;

        // ------------------------------ Parsing and validation of input args ---------------------------------
        if (!ParseAndCheckCommandLine(argc, argv)) {
            return 0;
        }

        // Setting EIS Message Bus publisher ----------------------
        const char* msg_bus_config = FLAGS_msg_bus.c_str();

        if(strlen(msg_bus_config) > 0 && FLAGS_alerts){
            //Loading JSON config file
            config_t* pub_config = json_config_new(msg_bus_config);
            if(pub_config == NULL) {
                LOG_ERROR_0("Failed to load JSON configuration");
                return -1;
            }

            std::condition_variable err_cv;

            g_input_queue = new MessageQueue(-1);
            g_publisher = new Publisher(
                    pub_config, err_cv, TOPIC, g_input_queue,
                    SERVICE_NAME);
            g_publisher->start();

            // Give time to initialize publisher
            std::this_thread::sleep_for(std::chrono::milliseconds(250));

        }

        readArea();

        std::string modelPath = FLAGS_m;
        std::size_t found = modelPath.find_last_of(".");
        if (found > modelPath.size()) {
            slog::info << "Invalid model name: " << modelPath << slog::endl;
            slog::info << "Expected to be <model_name>.xml" << slog::endl;
            return -1;
        }
        slog::info << "Model   path: " << modelPath << slog::endl;

        IEGraph::InitParams graphParams;
        graphParams.batchSize = FLAGS_bs;
        graphParams.maxRequests = FLAGS_nireq;
        graphParams.collectStats = FLAGS_show_stats;
        graphParams.reportPerf = FLAGS_pc;
        graphParams.modelPath = modelPath;
        graphParams.cpuExtPath = FLAGS_l;
        graphParams.cldnnConfigPath = FLAGS_c;
        graphParams.deviceName = FLAGS_d;

        std::shared_ptr<IEGraph> network(new IEGraph(graphParams));
        auto inputDims = network->getInputDims();
        if (4 != inputDims.size()) {
            throw std::runtime_error("Invalid network input dimensions");
        }

        std::vector<std::string> files;
        parseInputFilesArguments(files);

        slog::info << "\tNumber of input web cams:    " << FLAGS_nc << slog::endl;
        slog::info << "\tNumber of input video files: " << files.size() << slog::endl;
        slog::info << "\tDuplication multiplayer:     " << FLAGS_duplicate_num << slog::endl;

        const auto duplicateFactor = (1 + FLAGS_duplicate_num);
        size_t numberOfInputs = (FLAGS_nc + files.size()) * duplicateFactor;

        if (numberOfInputs == 0) {
            throw std::runtime_error("No valid inputs were supplied");
        }

        DisplayParams params = prepareDisplayParams(numberOfInputs);

        slog::info << "\tNumber of input channels:    " << numberOfInputs << slog::endl;
        if (numberOfInputs > MAX_INPUTS) {
            throw std::logic_error("Number of inputs exceed maximum value [25]");
        }

        VideoSources::InitParams vsParams;
        vsParams.queueSize = FLAGS_n_iqs;
        vsParams.collectStats = FLAGS_show_stats;
        vsParams.realFps = FLAGS_real_input_fps;
        vsParams.expectedHeight = static_cast<unsigned>(inputDims[2]);
        vsParams.expectedWidth = static_cast<unsigned>(inputDims[3]);

        VideoSources sources(vsParams);
        if (!files.empty()) {
            slog::info << "Trying to open input video ..." << slog::endl;
            for (auto &file : files){
                try{
                    sources.openVideo(file, false, FLAGS_loop_video);
                }
                catch (...){
                    slog::info << "Cannot open video [" << file << "]" << slog::endl;
                    throw;
                }
            }
        }
        if (FLAGS_nc) {
            slog::info << "Trying to connect " << FLAGS_nc << " web cams ..." << slog::endl;
            for (size_t i = 0; i < FLAGS_nc; ++i){
                try{
                    sources.openVideo(std::to_string(i), true, false);
                }
                catch (...){
                    slog::info << "Cannot open web cam [" << i << "]" << slog::endl;
                    throw;
                }
            }
        }
        sources.start();

        size_t currentFrame = 0;

        network->start([&](VideoFrame &img) {
            img.sourceIdx = currentFrame;
            auto camIdx = currentFrame / duplicateFactor;
            currentFrame = (currentFrame + 1) % numberOfInputs;
            return sources.getFrame(camIdx, img); }, [](InferenceEngine::InferRequest::Ptr req, const std::vector<std::string> &outputDataBlobNames, cv::Size frameSize) {
            auto output = req->GetBlob(outputDataBlobNames[0]);

            float* dataPtr = output->buffer();
            InferenceEngine::SizeVector svec = output->getTensorDesc().getDims();
            size_t total = 1;
            for (auto v : svec) {
                total *= v;
            }


            std::vector<Detections> detections(FLAGS_bs);
            for (auto& d : detections) {
                d.set(new std::vector<Detection>);
            }

            for (size_t i = 0; i < total; i+=7) {
                float conf = dataPtr[i + 2];
                float label = dataPtr[i + 1];
                if (conf > FLAGS_t) {
                    int idxInBatch = static_cast<int>(dataPtr[i]);
                    float x0 = std::min(std::max(0.0f, dataPtr[i + 3]), 1.0f);
                    float y0 = std::min(std::max(0.0f, dataPtr[i + 4]), 1.0f);
                    float x1 = std::min(std::max(0.0f, dataPtr[i + 5]), 1.0f);
                    float y1 = std::min(std::max(0.0f, dataPtr[i + 6]), 1.0f);

                    cv::Rect2f rect = {x0 , y0, x1-x0, y1-y0};
                    detections[idxInBatch].get<std::vector<Detection>>().emplace_back(rect, label, conf);
                }
            }
            return detections; });

        network->setDetectionConfidence(static_cast<float>(FLAGS_t));

        std::atomic<float> averageFps = {0.0f};

        std::vector<std::shared_ptr<VideoFrame>> batchRes;

        std::mutex statMutex;
        std::stringstream statStream;

        std::cout << "To close the application, press 'CTRL+C' here";
        if (!FLAGS_no_show) {
            std::cout << " or switch to the output window and press ESC key";
        }
        std::cout << std::endl;

        cv::Size graphSize{static_cast<int>(params.windowSize.width / 4), 60};
        Presenter presenter(FLAGS_u, params.windowSize.height - graphSize.height - 10, graphSize);
        const size_t outputQueueSize = 1;
        AsyncOutput output(FLAGS_show_stats, outputQueueSize,
                           [&](const std::vector<std::shared_ptr<VideoFrame>> &result) {
                               std::string str;
                               if (FLAGS_show_stats)
                               {
                                   std::unique_lock<std::mutex> lock(statMutex);
                                   str = statStream.str();
                               }
                               displayNSources(result, averageFps, str, params, presenter, &vehicle);
                               int key = cv::waitKey(1);
                               presenter.handleKey(key);

                               return (key != 27);
                           });

        output.start();

        using timer = std::chrono::high_resolution_clock;
        using duration = std::chrono::duration<float, std::milli>;
        timer::time_point lastTime = timer::now();
        duration samplingTimeout(FLAGS_fps_sp);

        size_t fpsCounter = 0;

        size_t perfItersCounter = 0;

        while (sources.isRunning() || network->isRunning()) {
            bool readData = true;
            while (readData) {
                auto br = network->getBatchData(params.frameSize);
                if (br.empty()){
                    break; // IEGraph::getBatchData had nothing to process and returned. That means it was stopped
                }
                for (size_t i = 0; i < br.size(); i++){
                    // this approach waits for the next input image for sourceIdx. If provided a single image,
                    // it may not show results, especially if -real_input_fps is enabled
                    auto val = static_cast<unsigned int>(br[i]->sourceIdx);
                    auto it = find_if(batchRes.begin(), batchRes.end(), [val](const std::shared_ptr<VideoFrame> &vf) { return vf->sourceIdx == val; });
                    if (it != batchRes.end()){
                        if (!FLAGS_no_show){
                            output.push(std::move(batchRes));
                        }
                        batchRes.clear();
                        readData = false;
                    }
                    batchRes.push_back(std::move(br[i]));
                }
            }
            ++fpsCounter;

            if (!output.isAlive()) {
                break;
            }

            auto currTime = timer::now();
            auto deltaTime = (currTime - lastTime);
            if (deltaTime >= samplingTimeout) {
                auto durMsec = std::chrono::duration_cast<duration>(deltaTime).count();
                auto frameTime = durMsec / static_cast<float>(fpsCounter);
                fpsCounter = 0;
                lastTime = currTime;

                if (FLAGS_no_show) {
                    slog::info << "Average Throughput : " << 1000.f / frameTime << " fps" << slog::endl;
                    if (++perfItersCounter >= FLAGS_n_sp){
                        break;
                    }
                }
                else{
                    averageFps = frameTime;
                }

                if (FLAGS_show_stats) {
                    auto inputStat = sources.getStats();
                    auto inferStat = network->getStats();
                    auto outputStat = output.getStats();

                    std::unique_lock<std::mutex> lock(statMutex);
                    statStream.str(std::string());
                    statStream << std::fixed << std::setprecision(1);
                    statStream << "Input reads: ";
                    for (size_t i = 0; i < inputStat.readTimes.size(); ++i) {
                        if (0 == (i % 4)) {
                            statStream << std::endl;
                        }
                        statStream << inputStat.readTimes[i] << "ms ";
                    }
                    statStream << std::endl;
                    statStream << "HW decoding latency: "
                               << inputStat.decodingLatency << "ms";
                    statStream << std::endl;
                    statStream << "Preprocess time: "
                               << inferStat.preprocessTime << "ms";
                    statStream << std::endl;
                    statStream << "Plugin latency: "
                               << inferStat.inferTime << "ms";
                    statStream << std::endl;

                    statStream << "Render time: " << outputStat.renderTime
                               << "ms" << std::endl;
                    statStream << "Mode: " << vehicle.get_mode_to_string() << std::endl;
                    if (FLAGS_show_calibration) {
                        for (int i = 0; i < MAX_INPUTS; i++) {
                            statStream << "Cam " << std::to_string(i + 1) << ": " << std::to_string(camDetections[i]) << std::endl;
                        }
                    }

                    if (FLAGS_no_show) {
                        slog::info << statStream.str() << slog::endl;
                    }
                }
            }
        }

        network.reset();

        std::cout << presenter.reportMeans() << '\n';

        // EIS Message Bus publisher
        if(strlen(msg_bus_config) > 0 && FLAGS_alerts){
            delete g_publisher;
            delete g_input_queue;
        }
    }
    catch (const std::exception &error)
    {
        slog::err << error.what() << slog::endl;
        return 1;
    }
    catch (...)
    {
        slog::err << "Unknown/internal exception happened." << slog::endl;
        return 1;
    }

    slog::info << "Execution successful" << slog::endl;
    return 0;
}