#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <vector>

struct Vec{
    //constructors
    Vec(float t_x=0.0f,float t_y=0.0f,float t_z=0.0f):x(t_x),y(t_y),z(t_z){};
    Vec(const Vec& t_vec):x(t_vec.x),y(t_vec.y),z(t_vec.z){};

    //use xyz or rgb variables
    union{float x; float r;};
    union{float y; float g;};
    union{float z; float b;};

    //add vectors
    Vec operator+(const Vec& t_vec) const{
        return Vec{x+t_vec.x,y+t_vec.y,z+t_vec.z};
    }

    //subtract vectors
    Vec operator-(const Vec& t_vec) const{
        return Vec{x-t_vec.x,y-t_vec.y,z-t_vec.z};
    }

    //reverse vector
    Vec operator-() const{
        return *this*-1;
    }

    //multiply by float scalar
    Vec operator*(float m) const{
        return Vec{x*m,y*m,z*m};
    }

    //Dot product
    float operator%(Vec t_vec) const{
        return x*t_vec.x+y*t_vec.y+z*t_vec.z;
    }

    //Cross product
    Vec operator^(Vec t_vec) const{
        return Vec{y*t_vec.z-z*t_vec.y,z*t_vec.x-x*t_vec.z,x*t_vec.y-y*t_vec.x};
    }

    float length() const{
        return std::sqrt(*this % *this);
    }

    Vec operator!() const{
        return (*this)*(1./length());
    }

    std::string str() const{
        return "[Vec x: "+std::to_string(x)+" y:"+std::to_string(y)+" z:"+std::to_string(z)+"]";
    }
};

struct Obj{
    Vec m_color;
    Obj():m_color(255,255,255){}
    void set_color(const Vec& t_col){
        m_color=t_col;
    }
    virtual bool is_hit_by_ray(const Vec& t_incoming_origin,
                               const Vec& t_incoming_dir,
                               float& hit_dist,
                               Vec& hit_color) const=0;
};

struct Tri : Obj {
    Vec p0,p1,p2;
    Vec u,v,norm;

    Tri(const Vec& t_p0,
        const Vec& t_p1,
        const Vec& t_p2):
        p0(t_p0),p1(t_p1),p2(t_p2){
        u=t_p1-t_p0;
        v=t_p2-t_p0;
        norm=v^u;
        //std::cout<<"New Tri: u:"<<u.str()<<" v:"<<v.str()<<std::endl;
    }

    bool is_hit_by_ray(const Vec& t_incoming_origin,
                               const Vec& t_incoming_dir,
                               float& hit_dist,
                               Vec& hit_color) const
    {

        if (norm%-t_incoming_dir < 0)
        {
            return false;

        }

        const float& pox{p0.x};
        const float& poy{p0.y};
        const float& poz{p0.z};
        const float& ux{u.x};
        const float& uy{u.y};
        const float& uz{u.z};
        const float& vx{v.x};
        const float& vy{v.y};
        const float& vz{v.z};
        const float& rx{t_incoming_dir.x};
        const float& ry{t_incoming_dir.y};
        const float& rz{t_incoming_dir.z};
        const float& ox{t_incoming_origin.x};
        const float& oy{t_incoming_origin.y};
        const float& oz{t_incoming_origin.z};

        /*
        std::cout<<"pox:"<<pox<<std::endl;
        std::cout<<"pox:"<<poy<<std::endl;
        std::cout<<"poy:"<<poz<<std::endl;

        std::cout<<"ux:"<<ux<<std::endl;
        std::cout<<"uy:"<<uy<<std::endl;
        std::cout<<"uz:"<<uz<<std::endl;

        std::cout<<"vx:"<<vx<<std::endl;
        std::cout<<"vy:"<<vy<<std::endl;
        std::cout<<"vz:"<<vz<<std::endl;

        std::cout<<"rx:"<<rx<<std::endl;
        std::cout<<"ry:"<<ry<<std::endl;
        std::cout<<"rz:"<<rz<<std::endl;

        std::cout<<"ox:"<<ox<<std::endl;
        std::cout<<"oy:"<<oy<<std::endl;
        std::cout<<"oz:"<<oz<<std::endl;
        */


        const float u_factor = (-(ox - pox)*(ry*vz - rz*vy) + (oy - poy)*(rx*vz - rz*vx) - (oz - poz)*(rx*vy - ry*vx))/(rx*uy*vz - rx*uz*vy - ry*ux*vz + ry*uz*vx + rz*ux*vy - rz*uy*vx);
        const float v_factor = ((ox - pox)*(ry*uz - rz*uy) - (oy - poy)*(rx*uz - rz*ux) + (oz - poz)*(rx*uy - ry*ux))/(rx*uy*vz - rx*uz*vy - ry*ux*vz + ry*uz*vx + rz*ux*vy - rz*uy*vx);
        const float ray_factor = (-(ox - pox)*(uy*vz - uz*vy) + (oy - poy)*(ux*vz - uz*vx) - (oz - poz)*(ux*vy - uy*vx))/(rx*uy*vz - rx*uz*vy - ry*ux*vz + ry*uz*vx + rz*ux*vy - rz*uy*vx);

        if(u_factor<0 or v_factor<0 or u_factor+v_factor>1 or ray_factor<0)
        {
            return false;
        }

        hit_dist=(t_incoming_dir*ray_factor).length();

        //double hit reflection correction
        if(hit_dist<=1e-6)
        {
            return false;
        }

        hit_color=m_color;
        return true;
    };
};

Vec get_ground_color(const Vec& t_origin, const Vec& t_dir){
    //check where ray hits floor
    float dist=-t_origin.y/t_dir.y;
    Vec floor_hit=t_origin+t_dir*dist;
    //checked floor pattern
    if((int)std::abs(std::floor(floor_hit.x))%2==(int)std::abs(std::floor(floor_hit.z))%2){
        return Vec{255,150,150};
    }else{
        return Vec{255,200,200};
    }
}

//get gradient sky color
Vec get_sky_color(const Vec& t_dir){
    return Vec{150,150,255}*std::pow(1-t_dir.y,2);
}


int main() {

    const int width=512;
    const int height=512;

    Vec ahead{0,0,1};
    Vec right{0.002,0,0};
    Vec up{0,0.002,0};


    //set up output file
    std::ofstream outputFile("img.ppm");
    outputFile << "P3" << " " << width << " " << height << " " << "255" << " ";



    std::vector<Obj*> scene_objects;

    scene_objects.push_back(new Tri{{-1,0,0},{1,0,0},{0,1.73,0}});
    scene_objects.back()->set_color({0,0,255});
    scene_objects.push_back(new Tri{{-3,0,10},{-2,0,10},{-1,1.73,10}});
    scene_objects.back()->set_color({0,255,0});

    //loop through output file
    for(int y=height/2; y>=-((height/2)-1); --y){
        for(int x=-((width/2)-1); x<=width/2; ++x){

            Vec color{0,0,0};

            Vec origin{0,1,-4};
            Vec dir=!Vec{right*(x-0.5)+up*(y-0.5)+ahead};

            bool obj_was_hit=false;
            float dist_to_hit;
            float min_hit_dist=std::numeric_limits<float>::max();
            Obj* closest_obj_ptr{nullptr};

            for(const auto& obj:scene_objects){
                if(obj->is_hit_by_ray(origin,dir,dist_to_hit,color))
                {
                    obj_was_hit=true;

                    if(dist_to_hit<min_hit_dist){
                        min_hit_dist=dist_to_hit;
                        closest_obj_ptr=obj;
                    }
                }
            }

            if(obj_was_hit) {
                closest_obj_ptr->is_hit_by_ray(origin,dir,dist_to_hit,color);
            }else{
                if (dir.y < 0) {
                    color = get_ground_color(origin, dir);
                } else {
                    color = get_sky_color(dir);
                }
            }


            outputFile << (std::max(0.f, std::min(color.r, 255.f))) << " "
                    << (std::max(0.f, std::min(color.g, 255.f))) << " "
                    << (std::max(0.f, std::min(color.b, 255.f))) << " ";
        }
    }

    outputFile.close();
    return EXIT_SUCCESS;
}