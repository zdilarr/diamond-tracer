#ifndef DIAMONDH
#define DIAMONDH

#include <algorithm>
#include <cmath>
#include "hitable.h"

/* Ray tracer class of a simplified diamond.    *
 *                                              *       
 *                                              *
 *                .---------.                   *
 *             .-/.          '-.                *
 *          .-' /             .\''-             *
 *        -' _.-               /.--.\           *
 *       /.-'   \.           ./     ./.         *
 *      / .      \.        ./       /.|         *
 *      \. .      \.------./.     ././          *
 *       \. \.   /.          \.  ././           *
 *         \. \./             \./ ./            *
 *          \.  \.-----------. /  /             *
 *            \  \           ./  /              *
 *             \. \          / ./               *
 *               \ \        /./                 *
 *                \.\      /./                  *
 *                  \.\   //                    *
 *                    \\ //                     *
 *                     \/                       *
 *                                              */

class diamond : public hitable {
public:
    diamond() {}
    diamond(vec3 cen, float h, material* m)
        : center(cen)
        , height(h)
        , mat_ptr(m){};
    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
    vec3 center;
    float height; /* Height from the bottom of the diamond to the center */
    material* mat_ptr;

private:
    static bool point_in_triangle(const vec3& a, const vec3& b, const vec3& c, const vec3& pt);
};

bool diamond::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{

    /* Reuse the sphere ray trace to determine if the   *
     * ray could intersect the diamond at all.          *
     * If the ray doesn't intersect the bounding sphere,*
     * then it can't intersect the diamond.             */

    vec3 oc = r.origin() - center; // vektor od pocetka zrake do centra kugle
    float a = dot(r.direction(), r.direction()); // intenzitet vektora kretanja zrake
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - height * height; // intenzitet vektora od pocetka zrake do centra kugle manje kvadrat precnika
    float discriminant = b * b - a * c; // nesto iz formule s wikipedije

    if (discriminant <= 0.) // If ray doesn't intersect sphere, it doesn't intersect diamond
        return false;

    float distance = std::min(
        (-b - sqrt(discriminant)) / a,
        (-b + sqrt(discriminant)) / a);

    if (distance <= 0.) // If sphere is behind the observer, diamond is behind observer
        return false; // (or the observer is inside the diamond), so ignore it

    //_____________________________

    //Checking:
    /*
	if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant))/a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / this->height;
            rec.mat_ptr = mat_ptr;
            return true;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / this->height;
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }
return false;*/

    /* Calculate vertices of a diamond and find an      *
     * intersection on one of the sides of the diamond. *
     *                                                  *
     * The top and bottom six points are arranged in a  *
     * counter-clockwise direction, starting from the   *
     * point with the largest x coordinate.             */

    vec3 bottom_pt(center.x(), center.y() - height, center.z());

    /* std::cout<<bottom_pt; */
    vec3 middle_pt[6];
    vec3 top_pt[6];
    vec3 center_top(center.x(), center.y() + (height / 2.), center.z());
    /* std::cout<<std::endl; */
    /*std::cout<<center_top; */

    vec3 nearest_pt;
    vec3 normal;
    bool found = false;
    const float PI = 3.14159265358979323846;

    for (int i = 0; i < 6; i++) {
        float angle = (PI * i) / 3.;

        middle_pt[i].e[0] /* x */ = center.x() + height * cos(angle);
        middle_pt[i].e[1] /* y */ = center.y();
        middle_pt[i].e[2] /* z */ = center.z() + height * (-sin(angle));

        top_pt[i].e[0] /* x */ = center.x() + (height / 2.) * cos(angle);
        top_pt[i].e[1] /* y */ = center.y() + (height / 2.);
        top_pt[i].e[2] /* z */ = center.z() + (height / 2.) * (-sin(angle));

        /* std::cout<<i<<"  "<<middle_pt[i]<<", "<<top_pt[i]<<std::endl;
	   std::cout<<i<<"  "<<middle_pt[i][0]<<", "<<middle_pt[i][1]<<", "<<middle_pt[i][2]<<std::endl;
	
	   std::cout<<i<<"  "<<top_pt[i][0]<<", "<<top_pt[i][1]<<", "<<top_pt[i][2]<<std::endl; */
    }

    const float epsilon = std::min(0.000001, fabs(0.000001 * height));

    //________________________________________________________

    /* Bottom sides (triangles):    */

    for (int i = 0; i < 6; ++i) {
        vec3 a = bottom_pt;
        vec3 b = middle_pt[i];
        vec3 c = middle_pt[(i + 1) % 6];

        /* Find the projection of the ray vector on the triangle plane: */
        vec3 line_pt = r.origin();
        vec3 line_vec = r.direction();
        vec3 plane_pt = a;
        vec3 plane_vec = cross(c - a, b - a);

        /* std::cout<<"Normala na ravan abc: "<<plane_vec<<std::endl;	*/

        //float numer = dot((line_pt - plane_pt), plane_vec);
        float numer = dot(-(line_pt - plane_pt), plane_vec);
        float denom = dot(line_vec, plane_vec);

        /* If line and plane are parallel, there's no projection.       */
        if (fabs(denom) < epsilon)
            continue;

        float d = numer / denom;

        /* Check if the distance is within the boundaries: */
        if ((line_vec * d).length() < t_min || (line_vec * d).length() > t_max)
            continue;

        vec3 proj = line_pt + (line_vec * d);

        /* Checking if points are OK:
		
		std::cout<<"Trokut: "<<a<<" * "<<b<<" * "<<c<<std::endl;
		std::cout<<"Projekcija: "<<proj<<std::endl;
		std::cout<<"Komplanarne: "<<dot((proj-a), cross((b-a),(c-a)))<<std::endl;
		int gg;
		std::cin>>gg;													*/

        /* If the point is not inside the triangle, ignore it.          */
        if (diamond::point_in_triangle(a, b, c, proj) == false)
            continue;

        /* If the point is inside the triangle, register it:            */
        if (found == false) {
            nearest_pt = proj;
            //normal = unit_vector(-line_vec + (plane_vec * 2 * dot(line_vec, plane_vec)));
            normal = unit_vector(plane_vec);
            found = true;
        }
        else {
            if ((proj - r.origin()).length() < (nearest_pt - r.origin()).length()) {
                nearest_pt = proj;
                //normal = unit_vector(-line_vec + (plane_vec * 2 * dot(line_vec, plane_vec)));
                normal = unit_vector(plane_vec);
            }
        }
    }

    //________________________________________________________

    /* Middle sides (rectangles):   */
    for (int i = 0; i < 6; ++i) {
        vec3 a = middle_pt[i];
        vec3 b = middle_pt[(i + 1) % 6];
        vec3 c = top_pt[(i + 1) % 6];
        vec3 d = top_pt[i];

        /* Find the projection of the ray vector on the triangle plane: */
        vec3 line_pt = r.origin();
        vec3 line_vec = r.direction();
        vec3 plane_pt = a;
        vec3 plane_vec = cross(b - a, c - a);
        /* std::cout<<"Normala na ravan abc: "<<plane_vec<<std::endl;	*/

        /* float numer = dot((line_pt - plane_pt), plane_vec);			*/
        float numer = dot(-(line_pt - plane_pt), plane_vec);
        float denom = dot(line_vec, plane_vec);

        /* If line and plane are parallel, there's no projection.       */
        if (fabs(denom) < epsilon)
            continue;

        float dd = numer / denom;

        /* Check if the distance is within the boundaries: */
        if ((line_vec * dd).length() < t_min || (line_vec * dd).length() > t_max)
            continue;

        vec3 proj = line_pt + (line_vec * dd);

        /* If the point is not inside the triangle, ignore it.          */
        if (diamond::point_in_triangle(a, b, c, proj) == false && diamond::point_in_triangle(c, d, a, proj) == false) {
            continue;
        }

        /* If the point is inside the triangle, register it:            */
        if (found == false) {
            nearest_pt = proj;
            //normal = unit_vector(-line_vec + (plane_vec * 2 * dot(line_vec, plane_vec)));
            normal = unit_vector(plane_vec);
            found = true;
        }
        else {
            if ((proj - r.origin()).length() < (nearest_pt - r.origin()).length()) {
                //normal = unit_vector(-line_vec + (plane_vec * 2 * dot(line_vec, plane_vec)));
                normal = unit_vector(plane_vec);
                nearest_pt = proj;
            }
        }
    }

    /* Top side (hexagon):                                              */
    {
        /* Find the projection of the ray vector on the triangle plane: */
        vec3 line_pt = r.origin();
        vec3 line_vec = r.direction();
        vec3 plane_pt = top_pt[0];
        vec3 plane_vec = cross(top_pt[0] - top_pt[1], top_pt[0] - top_pt[2]);
        /* std::cout<<"Normala na ravan abc: "<<plane_vec<<std::endl;	*/

        /* float numer = dot((line_pt - plane_pt), plane_vec);			*/
        float numer = dot(-(line_pt - plane_pt), plane_vec);
        float denom = dot(line_vec, plane_vec);

        /* If line and plane are parallel, there's no projection.       */
        if (fabs(denom) >= epsilon) {

            float d = numer / denom;

            /* Check if the distance is within the boundaries: */
            if ((line_vec * d).length() < t_min || (line_vec * d).length() > t_max)
                goto UPDATE;

            vec3 proj = line_pt + (line_vec * d);

            /* If the point is not inside the triangle, ignore it.          */
            bool inside = false;

            for (int i = 0; i < 6; ++i) {
                if (diamond::point_in_triangle(top_pt[i], top_pt[(i + 1) % 6], center_top, proj)) {
                    inside = true;
                    break;
                }
            }

            /* If the point is inside the triangle, register it.            */
            if (inside) {
                if (found == false) {
                    nearest_pt = proj;
                    //normal = unit_vector(-line_vec + (plane_vec * 2 * dot(line_vec, plane_vec)));
                    normal = unit_vector(plane_vec);
                    //std::cout<<"Normala na top hexagona: "<<normal<<std::endl;

                    found = true;
                }
                else {
                    if ((proj - r.origin()).length() < (nearest_pt - r.origin()).length()) {
                        nearest_pt = proj;
                        /* normal = unit_vector(-line_vec + (plane_vec * 2 * dot(line_vec, plane_vec))); */
                        normal = unit_vector(plane_vec);
                        /* std::cout<<"Normala na top hexagona: "<<normal<<std::endl;	*/
                    }
                }
            }
        }
    }

UPDATE:

    /* Update the intersection, if found:                               */
    if (found) {
        rec.t = (nearest_pt - r.origin()).length();
        rec.p = nearest_pt;
        rec.normal = normal;
        rec.mat_ptr = mat_ptr;
        return true;
    }

    /* None of the sides intersect the ray. */
    return false;
}

//______________________________________

bool diamond::point_in_triangle(const vec3& a, const vec3& b, const vec3& c, const vec3& pt)
{
    /* Check if projection is inside the triangle:                  */
    vec3 e = cross(pt - a, pt - b);
    vec3 f = cross(pt - b, pt - c);
    vec3 g = cross(pt - c, pt - a);

    /* If point is not inside the triangle, there's no intersection */
    return (dot(e, f) > 0. && dot(f, g) > 0. && dot(g, e) > 0.);
}

#endif
