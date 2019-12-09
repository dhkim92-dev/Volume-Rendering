#version 400

in vec3 EntryPoint; // 3d 볼륨 데이터의 좌표 
in vec4 ExitPointCoord; // 월드 스페이스 상의 엔트리포인트 좌표

uniform sampler1D TFF; // 볼륨데이터의 Intensity 에 매칭될 Transfer function
uniform sampler2D ExitPoints; // 좌표를 색상값으로 저장함
uniform sampler3D Volume; // 볼륨 데이터

uniform float Step_Size;
uniform vec2 Resolution;
uniform vec4 Bg_Color;// = vec4(1.0);

layout (location = 0) out vec4 FragColor;

void main()
{
    vec3 exit_point = texture(ExitPoints, gl_FragCoord.st/Resolution).xyz;

    // EntryPoint 가 exitPoint와 같다는 것은 배경이라는 의미이므로 폐기 
    // 프론트 페이스와 백 페이스를 그리는 큐브는 큐브의 좌표를 색상값으로 사용하므로 절대 같을 수 없다.
    if(EntryPoint == exit_point)
    {
        discard;
    }

    vec3 dir = exit_point - EntryPoint; // 레이 캐스트 진행 방향

    float len = length(dir); // 빛의 길이
    float actual_step_size = Step_Size * 0.5;

    vec3 d_dir = normalize(dir) * actual_step_size; // delta direction
    vec3 voxel_coord = EntryPoint; // 현재 볼륨의 값을 읽어올 좌표는 현재 프래그먼트에 매칭되는 오브젝트 상의 한 점의 월드 좌표
    vec4 color_accum = vec4(0.0);

    int num_steps = int(ceil(len/actual_step_size));

    for(int i = 0 ; i < num_steps ; i++)
    {
        float intensity = texture(Volume, voxel_coord).x;

        vec4 color_tff = texture(TFF, intensity);

        if(color_tff.a > 0.0){
            color_tff.a = 1.0 - pow(1.0 - color_tff.a, actual_step_size*200.0f);
            color_accum.rgb += (1.0 - color_accum.a) * color_tff.rgb * color_tff.a;
            color_accum.a += (1.0 - color_accum.a) * color_tff.a;
        }

        voxel_coord += d_dir;

        if(color_accum.a > 0.87){
            color_accum.a = 1.0;
            break;
        }
    }
    FragColor = mix(Bg_Color, color_accum, color_accum.a);

    //이 밑은 디버깅용 소스들입니다.
    //FragColor = vec4(dir,1.0);
    //FragColor = vec4(len, len, len, 1.0);
    //FragColor = vec4(actual_step_size, actual_step_size, actual_step_size, 1.0);
    //FragColor = vec4(num_steps%256, num_steps%256, num_steps%256, 1.0);
    //FragColor = vec4(d_dir,1.0);
    //FragColor = vec4(clamp(float(num_steps)/1500, 0, 1));
    //float intensity = texture(Volume, voxel_coord).x;
    //FragColor = vec4(exit_point,1.0);
    //FragColor = texture(TFF, 0.7);
    //FragColor = vec4(intensity, 0.0, 0.0, 1.0);

}
