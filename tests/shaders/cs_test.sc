#include <bgfx_compute.sh>

IMAGE2D_WO(s_output, rgba8, 0);

uniform vec4 u_color;

NUM_THREADS(8, 8, 1)
void main()
{
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
	imageStore(s_output, coord, u_color);
}
