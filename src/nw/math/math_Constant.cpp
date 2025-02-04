#include <nw/math/math_Constant.h>

#include <limits>

namespace nw { namespace math {

const f32 F_PI = 3.141592653589793f;
const u32 I_HALF_ROUND_IDX = 0x80000000;
const f32 F_ULP = std::numeric_limits<f32>::epsilon() * 2.0f; // 2.384185791015625E-07f

} } // namespace nw::math
