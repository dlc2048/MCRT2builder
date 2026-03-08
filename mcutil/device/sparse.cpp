
#include <sutil/Exception.h>
#include <sutil/sutil.h>

#include "sparse.hpp"


#define CHECK_CUSPARSE(func)                                                   \
{                                                                              \
    cusparseStatus_t status = (func);                                          \
    if (status != CUSPARSE_STATUS_SUCCESS) {                                   \
        printf("CUSPARSE API failed at line %d with error: %s (%d)\n",         \
               __LINE__, cusparseGetErrorString(status), status);              \
        throw sutil::Exception( cusparseGetErrorString(status) );              \
    }                                                                          \
}


namespace mcutil {


    DenseToCOOSparseHandler::DenseToCOOSparseHandler(size_t size, float* dense_ptr, int* coo_index_ptr, float* coo_val_ptr) {
        CHECK_CUSPARSE(cusparseCreate(&this->_handle));

        // dense matrix
        CHECK_CUSPARSE(cusparseCreateDnMat(
            &this->_mat_dense, 
            1,
            size, 
            size, 
            dense_ptr,
            CUDA_R_32F, 
            CUSPARSE_ORDER_ROW
        ));

        // set inner device row array
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&this->_d_row), sizeof(int) * size));

        // sparse matrix
        CHECK_CUSPARSE(cusparseCreateCoo(
            &this->_mat_sparse, 
            1, 
            size,
            0,
            this->_d_row, 
            coo_index_ptr, 
            coo_val_ptr,
            CUSPARSE_INDEX_32I, 
            CUSPARSE_INDEX_BASE_ZERO, 
            CUDA_R_32F
        ));

        // prepare buffer
        size_t buffer_size;
        CHECK_CUSPARSE(cusparseDenseToSparse_bufferSize(
            this->_handle, 
            this->_mat_dense, 
            this->_mat_sparse, 
            CUSPARSE_DENSETOSPARSE_ALG_DEFAULT, 
            &buffer_size
        ));

        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&this->_buffer), buffer_size));
        this->_memoryUsageAppend(buffer_size);

        return;
    }


    DenseToCOOSparseHandler::~DenseToCOOSparseHandler() {
        CHECK_CUSPARSE(cusparseDestroyDnMat(this->_mat_dense));
        CHECK_CUSPARSE(cusparseDestroySpMat(this->_mat_sparse));
        CHECK_CUSPARSE(cusparseDestroy(this->_handle));
        CUDA_CHECK(cudaFree(this->_buffer));
        CUDA_CHECK(cudaFree(this->_d_row));
    }


    size_t DenseToCOOSparseHandler::convert() {
        CHECK_CUSPARSE(cusparseDenseToSparse_analysis(
            this->_handle,
            this->_mat_dense,
            this->_mat_sparse, 
            CUSPARSE_DENSETOSPARSE_ALG_DEFAULT, 
            this->_buffer
        ));
        // get number of non-zero elements
        int64_t nr, nc, nnz;
        CHECK_CUSPARSE(cusparseSpMatGetSize(
            this->_mat_sparse,
            &nr, &nc, &nnz
        ));

        // execute conversion
        CHECK_CUSPARSE(cusparseDenseToSparse_convert(
            this->_handle, 
            this->_mat_dense, 
            this->_mat_sparse, 
            CUSPARSE_DENSETOSPARSE_ALG_DEFAULT, 
            this->_buffer
        ));

        return (size_t)nnz;
    }


}