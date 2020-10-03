// Copyright (c) 2019 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @file
 * @brief High level video frame abstraction.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#ifndef _EIS_UDF_FRAME_H
#define _EIS_UDF_FRAME_H

#include <atomic>

#include <eis/msgbus/msg_envelope.h>
#include <eis/utils/logger.h>

namespace eis {
namespace udf {

/**
 * Encoding types for the given frame.
 */
enum EncodeType {
    NONE,
    JPEG,
    PNG,
};

/**
 * Wrapper around a frame object
 */
class Frame : public eis::msgbus::Serializable {
private:
    // This pointer represents the underlying object in which the raw pixel
    // data for the frame resides. This pointer could be a GstBuffer, cv::Mat
    // object, or any other representation. The purpose of having this pointer
    // is to keep the memory of the underlying frame alive while the user needs
    // to access the underlying bytes for the frame.
    void* m_frame;

    // Underlying free method for the frame
    void (*m_free_frame)(void*);

    // This pointer points to the underlying bytes for the frame, i.e. the
    // bytes for the raw pixels of the frame. Note that the memory for this
    // void* is ultimatly residing in the m_frame's memory, this is just a
    // pointer to that underlying data provided to the constructor.
    void* m_data;

    // This is only used when the frame was deserialized from a
    // msg_envelope_t and then reserialized. This keeps track of the actual
    // underlying blob memory (i.e. the frame's pixel data).
    owned_blob_t* m_blob_ptr;

    // Meta-data associated with the frame
    msg_envelope_t* m_meta_data;

    // Must-have attributes
    int m_width;
    int m_height;
    int m_channels;

    // Flag for if the frame has been serailized already
    std::atomic<bool> m_serialized;

    // Encoding type for the frame
    EncodeType m_encode_type;

    // Encoding level for the frame
    int m_encode_level;

    /**
     * Private helper function to encode the given frame during serialization.
     */
    void encode_frame();

    /**
     * Function to be passed to the EIS Message Bus for freeing the frame after
     * it has been transmitted over the bus.
     */
    static void msg_free_frame(void* hint) {
        LOG_DEBUG("Freeing frame...");
        // Cast to a frame pointer
        Frame* frame = (Frame*) hint;

        // Free frame data (if given a free function)
        if(frame->m_free_frame != NULL) {
            LOG_DEBUG("Calling m_free_frame");
            frame->m_free_frame(frame->m_frame);
        }

        // Free the owned blob for the frame data if this was deserialized from
        // a msg_envelope_t
        if(frame->m_blob_ptr != NULL) {
            owned_blob_destroy(frame->m_blob_ptr);
        }

        delete frame;
    };

    /**
     * Private @c Frame copy constructor.
     *
     * \note The constructor is copied, because this object is not supposed to
     *      be copied. It is important to note that this only blocks callers
     *      from passing the @c Frame by reference rather than as a pointer.
     */
    Frame(const Frame& src);

public:
    /**
     * Constructor
     *
     * @param frame             - Underlying frame object
     * @param width             - Frame width
     * @param height            - Frame height
     * @param data              - Constant pointer to the underlying frame data
     * @param free_frame        - Function to free the underlying frame
     * @param encode            - (Optional) Frame encoding type
     *                            (default:  @c EncodeType:NONE)
     * @param encode_level      - (Optional) Encode level
     *                            (value depends on encoding type)
     */
    Frame(void* frame, int width, int height, int channels, void* data,
            void (*free_frame)(void*), EncodeType encode=EncodeType::NONE,
            int encode_level=0);

    /**
     * Deserialize constructor
     *
     * @param msg - Message envelope to deserialize
     */
    Frame(msg_envelope_t* msg);

    /**
     * Destructor
     */
    ~Frame();

    /**
    * Get frame encoding type
    *
    * @return EncodeType
    */
    EncodeType get_encode_type();

    /**
    * Get frame encoding level
    * @return int
    */
    int get_encode_level();

    /**
     * Get frame width.
     *
     * @return int
     */
    int get_width();

    /**
     * Get frame height.
     *
     * @return int
     */
    int get_height();

    /**
     * Get number of channels in the frame.
     *
     * @return int
     */
    int get_channels();

    /**
     * Get the underlying frame data.
     *
     * @return void*
     */
    void* get_data();

    /**
     * Set new data on the frame.
     *
     * @param frame             - Underlying frame object
     * @param width             - Frame width
     * @param height            - Frame height
     * @param data              - Constant pointer to the underlying frame data
     * @param free_frame        - Function to free the underlying frame
     */
    void set_data(void* frame, int width, int height, int channels, void* data,
                  void (*free_frame)(void*));

    /**
     * Set the encoding for the frame.
     *
     * @param enc_type - Encoding type
     * @param enc_lvl  - Encoding level
     */
    void set_encoding(EncodeType enc_type, int enc_lvl);

    /**
     * Get @c msg_envelope_t meta-data envelope.
     *
     * \note NULL will be returned if the frame has already been serialized.
     *
     * @return @c msg_envelope_t*
     */
    msg_envelope_t* get_meta_data();

    /**
     * \note **IMPORTANT NOTE:** This method PERMANENTLY changes the frame
     *      object and can only be called ONCE. The reson for this is to make
     *      sure that all of the underlying memory is properly managed and
     *      free'ed at a point where the application will not SEGFAULT due to a
     *      double free situation. All methods (except for width(), height(),
     *      and channels()) will return errors after this.
     *
     * Overriden serialize method.
     *
     * @return @c msg_envelope_t*
     */
    msg_envelope_t* serialize() override;
};

} // udf
} // eis

#endif // _EIS_UDF_FRAME_H
