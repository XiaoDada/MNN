//
//  onnOpConverter.cpp
//  MNNConverter
//
//  Created by MNN on 2019/01/31.
//  Copyright © 2018, Alibaba Group Holding Limited
//

#include "onnxOpConverter.hpp"

onnxOpConverterSuit::onnxOpConverterSuit() {
}

onnxOpConverterSuit::~onnxOpConverterSuit() {
    for (auto& iter : mConverterContainer) {
        delete iter.second;
    }
    mConverterContainer.clear();
}

onnxOpConverterSuit* onnxOpConverterSuit::global = nullptr;

onnxOpConverterSuit* onnxOpConverterSuit::get() {
    if (global == nullptr) {
        global = new onnxOpConverterSuit;
    }
    return global;
}

void onnxOpConverterSuit::insert(onnxOpConverter* t, const char* name) {
    mConverterContainer.insert(std::make_pair(name, t));
}

onnxOpConverter* onnxOpConverterSuit::search(const std::string& name) {
    auto iter = mConverterContainer.find(name);
    if (iter == mConverterContainer.end()) {
        return nullptr;
    }
    return iter->second;
}
MNN::BlobT* onnxOpConverter::convertTensorToBlob(const onnx::TensorProto * constantTp) {
    auto constantParam = new MNN::BlobT;
    auto dataType = convertDataType(constantTp->data_type());
    
    constantParam->dataType   = dataType;
    constantParam->dataFormat = MNN::MNN_DATA_FORMAT_NCHW;
    
    size_t dimSize = constantTp->dims().size();
    constantParam->dims.resize(dimSize);
    size_t dataSize = 1;
    for (int i = 0; i < dimSize; ++i) {
        constantParam->dims[i] = constantTp->dims(i);
        dataSize               = dataSize * constantTp->dims(i);
    }
    
    const void *tensor_content = nullptr;
    if (dataSize == 1 || dimSize == 0) {
        // scalar or one dim data(only one data)
        switch (dataType) {
            case MNN::DataType_DT_INT64:
                tensor_content = constantTp->int64_data().data();
                break;
            case MNN::DataType_DT_INT32:
                tensor_content = constantTp->int32_data().data();
                break;
            default:
                tensor_content = constantTp->float_data().data();
                break;
        }
        // some Const node is Scalar, but must
        // access to data from tensor_content
        if (!tensor_content) {
            tensor_content = constantTp->raw_data().data();
        }
        
    } else {
        tensor_content = constantTp->raw_data().data();
    }
    if (!tensor_content) {
        DLOG(FATAL) << "Convert no data, "
        "Please make sure ";
    }
    
    switch (constantTp->data_type()) {
        case onnx::TensorProto_DataType_INT64: {
            constantParam->int32s.resize(dataSize);
            auto source = (int64_t *)tensor_content;
            
            for (int i = 0; i < dataSize; ++i) {
                constantParam->int32s[i] = source[i];
            }
            break;
        }
        case onnx::TensorProto_DataType_INT32: {
            auto source = (int32_t *)tensor_content;
            constantParam->int32s.resize(dataSize);
            for (int i = 0; i < dataSize; ++i) {
                constantParam->int32s[i] = source[i];
            }
            break;
        }
        case onnx::TensorProto_DataType_UINT8: {
            auto source = (uint8_t *)tensor_content;
            constantParam->uint8s.resize(dataSize);
            for (int i = 0; i < dataSize; ++i) {
                constantParam->uint8s[i] = source[i];
            }
            break;
        }
        default: {
            float *tempFloatData = (float *)tensor_content;
            constantParam->float32s.resize(dataSize);
            for (int i = 0; i < dataSize; ++i) {
                constantParam->float32s[i] = tempFloatData[i];
            }
            break;
        }
    }
    return constantParam;
}
