#include "Math.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/gtc/epsilon.hpp"

namespace Iris::Math {
    /// Make a linear combination of two vectors and return the result.
    // result = (a * ascl) + (b * bscl)
    template <typename T, glm::qualifier Q>
    GLM_FUNC_QUALIFIER glm::vec<3, T, Q> combine(
            glm::vec<3, T, Q> const& a,
            glm::vec<3, T, Q> const& b,
            T ascl, T bscl) {
        return (a * ascl) + (b * bscl);
    }

    template <typename T, glm::qualifier Q>
    GLM_FUNC_QUALIFIER glm::vec<3, T, Q> scale(glm::vec<3, T, Q> const& v, T desiredLength) {
        return v * desiredLength / length(v);
    }

    bool DecomposeTransform(glm::mat4 transform, glm::vec3& outTranslate, glm::vec3& outRotation, glm::vec3& outScale) {
        using namespace glm;
        using T = float;
        mat<4, 4, T> LocalMatrix(transform);
        vec<4, T> Skew{};

        // Normalize the matrix.
        if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
            return false;

        for (length_t i = 0; i < 4; ++i)
            for (length_t j = 0; j < 4; ++j)
                LocalMatrix[i][j] /= LocalMatrix[3][3];

        // perspectiveMatrix is used to solve for perspective, but it also provides
        // an easy way to test for singularity of the upper 3x3 component.
        mat<4, 4, T> PerspectiveMatrix(LocalMatrix);

        for (length_t i = 0; i < 3; i++)
            PerspectiveMatrix[i][3] = static_cast<T>(0);
        PerspectiveMatrix[3][3] = static_cast<T>(1);

        /// TODO: Fixme!
        if (epsilonEqual(determinant(PerspectiveMatrix), static_cast<T>(0), epsilon<T>()))
            return false;

        // Next take care of translation (easy).
        outTranslate = vec<3, T>(LocalMatrix[3]);
        LocalMatrix[3] = vec<4, T>(0, 0, 0, LocalMatrix[3].w);

        vec<3, T> Row[3], Pdum3{};

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        outScale.x = length(Row[0]);// v3Length(Row[0]);

        Row[0] = scale(Row[0], static_cast<T>(1));

        // Compute XY shear factor and make 2nd row orthogonal to 1st.
        Skew.z = dot(Row[0], Row[1]);
        Row[1] = combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);

        // Now, compute Y scale and normalize 2nd row.
        outScale.y = length(Row[1]);
        Row[1] = scale(Row[1], static_cast<T>(1));

        // Compute XZ and YZ shears, orthogonalize 3rd row.
        Skew.y = glm::dot(Row[0], Row[2]);
        Row[2] = combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
        Skew.x = glm::dot(Row[1], Row[2]);
        Row[2] = combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);

        // Next, get Z scale and normalize 3rd row.
        outScale.z = length(Row[2]);
        Row[2] = scale(Row[2], static_cast<T>(1));
        Skew.y /= outScale.z;
        Skew.x /= outScale.z;

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0) {
            for (length_t i = 0; i < 3; i++) {
                outScale[i] *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }


        outRotation.y = asin(-Row[0][2]);
        if (cos(outRotation.y) != 0) {
            outRotation.x = atan2(Row[1][2], Row[2][2]);
            outRotation.z = atan2(Row[0][1], Row[0][0]);
        } else {
            outRotation.x = atan2(-Row[2][0], Row[1][1]);
            outRotation.z = 0;
        }

        return true;
    }
}