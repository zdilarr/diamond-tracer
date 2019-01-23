#include <iostream>
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "material.h"
#include <cfloat>
#include <fstream>
#include "diamond.h"

#define OUTPUT fout

vec3 color(const ray& r, hitable* world, int depth)
{
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * color(scattered, world, depth + 1);
        }
        else {
            return vec3(0, 0, 0);
        }
    }
    else {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
    }
}

hitable* random_scene()
{
    int n = 500;
    hitable** list = new hitable*[n + 1];
    list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            //float choose_mat = (float)rand() / RAND_MAX;
            //float choose_mat = 0.2;

            vec3 center(a + 0.9 * (float)rand() / RAND_MAX, 0.2, b + 0.9 * (float)rand() / RAND_MAX);
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                list[i++] = new diamond(center, 0.2, new lambertian(vec3(0.2, 0.2, 0.2)));
            }
        }
    }

    list[i++] = new diamond(vec3(0, 1, 0), 1.0, new dielectric(2.4));
    list[i++] = new diamond(vec3(-4, 1, 0), 1.0, new metal(vec3(0.0, 0.9, 0.9), 0.0));
    list[i++] = new diamond(vec3(4, 1, 0), 1.0, new dielectric(2.4));

    return new hitable_list(list, i);
}

int main()
{
    std::ofstream fout;
    fout.open("image.ppm");
    int nx = 1024;
    int ny = 768;
    int ns = 10;
    OUTPUT << "P3\n" << nx << " " << ny << "\n255\n";

    hitable* world = random_scene();

    vec3 lookfrom(100, 100, 100);
    vec3 lookat(2, 1, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;

    camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus);

    for (int j = ny - 1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            for (int s = 0; s < ns; s++) {
                float u = float(i + (float)rand() / RAND_MAX) / float(nx);
                float v = float(j + (float)rand() / RAND_MAX) / float(ny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r, world, 0);
            }
            col /= float(ns);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);
            OUTPUT << ir << " " << ig << " " << ib << "\n";
        }
    }
}
