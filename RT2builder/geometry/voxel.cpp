
#include "voxel.hpp"


namespace geo {


    Voxel::Voxel() :
        _shape(make_int3(0, 0, 0)),
        mcutil::Affine()
    {}


    Voxel::Voxel(const std::string& file_name) :
        _shape ({ 0, 0, 0 }),
        mcutil::Affine()
    {
        mcutil::FortranIfstream file(file_name);
        if (file.fail())
            mclog::fatalFileNotExist(file_name);

        // read dimension
        std::vector<int>    shape;
        std::vector<double> affine;
        try {
            shape  = file.read<int>();
            affine = file.read<double>();
            this->_shape  = { shape[0],  shape[1],  shape[2]  };
            this->transform(mcutil::Affine(
                affine[0], affine[1], affine[2],  affine[3],
                affine[4], affine[5], affine[6],  affine[7],
                affine[8], affine[9], affine[10], affine[11]
            ));
            file.read<int>();  // sparse
        }
        catch (std::exception& e) {
            mclog::fatal() << std::string(e.what());
        }

        // read value
        this->_array = file.read<uint16_t>();
        if (this->_array.size() != (size_t)this->_shape.x * this->_shape.y * this->_shape.z)
            mclog::fatal() << "Voxel dimension data is corrupted";

        // read region
        int n_region = file.read<int>()[0];
        for (int i = 0; i < n_region; ++i) {
            std::vector<char> reg_char = file.read<char>();
            std::string region(reg_char.begin(), reg_char.end());
            this->_region.push_back(region);
        }
    }


    /*
    Voxel::Voxel(
        const std::string& file_name,
        const std::deque<Hounsfield>& hounsfield) :
        _origin (make_double3(0, 0, 0)),
        _size   (make_double3(0, 0, 0)),
        _shape  (make_int3   (0, 0, 0)) 
    {
        mcutil::FortranIfstream file(file_name);
        if (file.fail())
            mclog::fatalFileNotExist(file_name);

        // read dimension
        std::vector<double> coeff;
        std::vector<double> size;
        std::vector<double> origin;
        std::vector<int>    shape;
        try {
            coeff  = file.read<double>();
            origin = file.read<double>();
            size   = file.read<double>();
            shape  = file.read<int>();
            this->_origin = { origin[0], origin[1], origin[2] };
            this->_size   = { size[0], size[1], size[2] };
            this->_shape  = { shape[0], shape[1], shape[2] };
        }
        catch (std::exception& e) {
            mclog::fatal(std::string(e.what()));
        }

        // check hounsfield deque is sorted
        for (size_t i = 0; i < hounsfield.size(); ++i) {
            if (i) if (hounsfield[i - 1].ceil() > hounsfield[i].ceil()) {
                mclog::fatal("ceil of 'HOUNS' card must be in increasing order");
            }
            this->_region.push_back(hounsfield[i].region());
        }

        // read value
        std::vector<uint16_t> pdata = file.read<uint16_t>();
        if (pdata.size() != (size_t)this->_shape.x * this->_shape.y * this->_shape.z)
            mclog::fatal("Voxel dimension data is corrupted");

        this->_array.resize(pdata.size());

        bool in_range       = true;
        int  houns_exceeded = -99999;
        for (size_t i = 0; i < pdata.size(); ++i) {
            int value = ((double)pdata[i] + coeff[0]) / coeff[1];
            bool exceeded = true;
            for (int j = 0; j < hounsfield.size(); ++j) {
                if (value < hounsfield[j].ceil()) {
                    exceeded = false;
                    this->_array[i] = j;
                    break;
                }
                if (exceeded)
                    houns_exceeded = value;
            }
            
            in_range &= !exceeded;
        }
        if (!in_range) {
            std::stringstream ss;
            ss << "HU value " << houns_exceeded << " out of range" << std::endl;
            mclog::fatal(ss);
        }
    }
    */


    std::vector<geo::MarkedTriangle> Voxel::getInterior() const {
        std::vector<MarkedTriangle> vin;
        double3 size = this->scale();

        size_t oy, oz;

        if (this->determinant() > 0) {  // Face orientation
            oy = 1;
            oz = 2;
        }
        else {
            oy = 2;
            oz = 1;
        }

        for (size_t i = 0; i < this->_shape.x; ++i) {
            for (size_t j = 0; j < this->_shape.y; ++j) {
                for (size_t k = 0; k < this->_shape.z; ++k) {
                    double x, y, z;
                    x = (double)i - 0.5;
                    y = (double)j - 0.5;
                    z = (double)k - 0.5;
                    int current_reg = this->_array[(i * this->_shape.y + j) * this->_shape.z + k];
                    // z plane
                    if (k) {
                        MarkedTriangle zm1, zm2;
                        int idx_front, idx_back;
                        idx_front = this->_array[(i * this->_shape.y + j) * this->_shape.z + k - 1];
                        idx_back = current_reg;

                        if (idx_back != idx_front) {
                            zm1.region_idx_backface  = idx_back;
                            zm2.region_idx_backface  = idx_back;
                            zm1.region_idx_frontface = idx_front;
                            zm2.region_idx_frontface = idx_front;

                            zm1.vertex[0]  = make_float3(mcutil::transform({ x,     y,     z }, *this));
                            zm1.vertex[oy] = make_float3(mcutil::transform({ x + 1, y + 1, z }, *this));
                            zm1.vertex[oz] = make_float3(mcutil::transform({ x + 1, y,     z }, *this));
                            zm2.vertex[0]  = make_float3(mcutil::transform({ x,     y,     z }, *this));
                            zm2.vertex[oy] = make_float3(mcutil::transform({ x,     y + 1, z }, *this));
                            zm2.vertex[oz] = make_float3(mcutil::transform({ x + 1, y + 1, z }, *this));

                            vin.push_back(zm1);
                            vin.push_back(zm2);
                        }
                    }
                    // y plane
                    if (j) {
                        MarkedTriangle ym1, ym2;
                        int idx_front, idx_back;
                        idx_front = this->_array[(i * this->_shape.y + j - 1) * this->_shape.z + k];
                        idx_back = current_reg;

                        if (idx_back != idx_front) {
                            ym1.region_idx_backface  = idx_back;
                            ym2.region_idx_backface  = idx_back;
                            ym1.region_idx_frontface = idx_front;
                            ym2.region_idx_frontface = idx_front;

                            ym1.vertex[0]  = make_float3(mcutil::transform({ x,     y, z     }, *this));
                            ym1.vertex[oy] = make_float3(mcutil::transform({ x + 1, y, z + 1 }, *this));
                            ym1.vertex[oz] = make_float3(mcutil::transform({ x,     y, z + 1 }, *this));
                            ym2.vertex[0]  = make_float3(mcutil::transform({ x,     y, z     }, *this));
                            ym2.vertex[oy] = make_float3(mcutil::transform({ x + 1, y, z     }, *this));
                            ym2.vertex[oz] = make_float3(mcutil::transform({ x + 1, y, z + 1 }, *this));

                            vin.push_back(ym1);
                            vin.push_back(ym2);
                        }
                    }
                    // x plane
                    if (i) {
                        MarkedTriangle xm1, xm2;
                        int idx_front, idx_back;
                        idx_front = this->_array[((i - 1) * this->_shape.y + j) * this->_shape.z + k];
                        idx_back = current_reg;

                        if (idx_back != idx_front) {
                            xm1.region_idx_backface  = idx_back;
                            xm2.region_idx_backface  = idx_back;
                            xm1.region_idx_frontface = idx_front;
                            xm2.region_idx_frontface = idx_front;

                            xm1.vertex[0]  = make_float3(mcutil::transform({ x, y,     z     }, *this));
                            xm1.vertex[oy] = make_float3(mcutil::transform({ x, y + 1, z + 1 }, *this));
                            xm1.vertex[oz] = make_float3(mcutil::transform({ x, y + 1, z     }, *this));
                            xm2.vertex[0]  = make_float3(mcutil::transform({ x, y,     z     }, *this));
                            xm2.vertex[oy] = make_float3(mcutil::transform({ x, y,     z + 1 }, *this));
                            xm2.vertex[oz] = make_float3(mcutil::transform({ x, y + 1, z + 1 }, *this));

                            vin.push_back(xm1);
                            vin.push_back(xm2);
                        }
                    }
                }
            }
        }
        return vin;
    }


    int3 Voxel::shape() const {
        return this->_shape;
    }


    const std::vector<uint16_t>& Voxel::array() const {
        return this->_array;
    }


    const std::vector<std::string>& Voxel::region() const {
        return this->_region;
    }


    float3 make_float3(double3 point) {
        return { (float)point.x, (float)point.y, (float)point.z };
    }

}