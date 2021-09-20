layout(points) in;
layout(triangle_strip, max_vertices = 96) out;

uniform float radius;

in mat2 vJ[1];

out vec3 color;

const float pi = 3.14159265359;
const float twoPi = 6.28318530718;

void main(void) {
    vec2 center = gl_in[0].gl_Position.xy;

    mat2 J = vJ[0];

    for (int i = 0; i < 32; i++) {
        // TASK 4: use the index to set the color based on where we are in the circle
        color = vec3(0.106,0.620,0.467);

        gl_Position = vec4(center, 0, 1);  // first point in triangle is in the origin of the circle
        EmitVertex();

        float a1 = twoPi * (i / 32.0);        // angle between y-axis and second point in the triangle
        float a2 = twoPi * ((i + 1) / 32.0);  // angle between y-axis and third point in the triangle

        vec2 o1 = vec2(0);  // calculate an offset vector for the second point in the triangle based on a1

        gl_Position = vec4(center + o1 * radius, 0, 1);
        EmitVertex();

        vec2 o2 = vec2(0);  // calculate an offset vector for the second point in the triangle based on a2

        gl_Position = vec4(center + o2 * radius, 0, 1);
        EmitVertex();

        EndPrimitive();
    }
}
