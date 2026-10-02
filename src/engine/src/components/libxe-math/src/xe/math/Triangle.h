
#ifndef __XE_MATH_TRIANGLE_HPP__
#define __XE_MATH_TRIANGLE_HPP__

#include "Plane.h"
#include "Ray.h"
#include "Vector.h"
#include <array>

namespace xe {
    template <typename T> struct TTriangle {
        TVector<T, 3> P1;
        TVector<T, 3> P2;
        TVector<T, 3> P3;

        TTriangle() {
        }

        TTriangle(const TVector<T, 3> &p1, const TVector<T, 3> &p2, const TVector<T, 3> &p3) {
            P1 = p1;
            P2 = p2;
            P3 = p3;
        }

        TVector<T, 3> computeNormal() const {
            return normalize(computeNormalUnnormalized());
        }

        TVector<T, 3> computeNormalUnnormalized() const {
            return cross((P2 - P1), P3 - P1);
        }
    };

    using Triangle = TTriangle<float>;
    using Triangled = TTriangle<double>;

    extern template struct TTriangle<float>;
    extern template struct TTriangle<double>;

    /*
    template<typename T=float>
    TVector3<T> hit(const TTriangle<T> &tri, const TRay<T>& ray) {
        // Generar un plano con los tres puntos
        auto plane = planeTriangle<T>(tri.P1, tri.P2, tri.P3);

        if (plane.intersect(ray, &info) == true) {
            if (info.distance <= 0.0) {
                continue;
            }

            // Comprobar si el punto de interseccion pertenece al triangulo o no, usando
            // coordenadas baricentricas
            Vector3 r0=ray.getPointAt(info.distance);
            Vector3 p=ray.getPoint(), q=r0;
            Vector3 a=tri.p1, b=tri.p2, c=tri.p3;

            Vector3 pq = (q - p);
            Vector3 pa = (a - p);
            Vector3 pb = (b - p);
            Vector3 pc = (c - p);

            float u, v, w;
            u = pq.triple(pc, pb);
            v = pq.triple(pa, pc);
            w = pq.triple(pb, pa);

            // Detectar si existe colision, sin importar como esten ordenados los triangulos
            bool isBackSide = (u > 0.0f && v > 0.0f && w > 0.0f);
            bool isFrontSide = (u < 0.0f && v < 0.0f && w < 0.0f);
        }
    }
     */
} // namespace xe

#endif
