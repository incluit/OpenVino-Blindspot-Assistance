# Create variables for all models used by the tutorials to make 
#  it easier to reference them with short names

# check for variable set by setupvars.sh in the SDK, need it to find models
: ${InferenceEngine_DIR:?"Must source the setupvars.sh in the SDK to set InferenceEngine_DIR"}

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

PROJECT_PATH=$parent_path/../
modelDir=$parent_path/../models/

modName=pedestrian-and-vehicle-detector-adas-0001
export pv16=$modelDir/FP16/$modName.xml
export pv32=$modelDir/FP32/$modName.xml

modName=instance-segmentation-security-0050
export is16=$modelDir/FP16/$modName.xml
export is32=$modelDir/FP32/$modName.xml

export PROJECT_PATH=${PROJECT_PATH}
