#include "minirt.h"

/*
ambient lighting
*/

t_light *point_light(t_vec orig, t_color color, double intensity)
{
    t_light *new;

    new = (t_light*)malloc(sizeof(t_light));
    new->orig = orig;
    new->color = color;
    new->intensity = intensity;
    return (new);
}

t_color phong_lighting(t_scene *scene)
{
    t_color light_color;
    t_obj *light;

    light_color = color(0, 0, 0);
    light = scene->light.next;
    while (light)
    {
        light_color = vec_sum(light_color, point_light_get(scene, light->element));
        light = light->next;
    }
    light_color = vec_sum(light_color, scene->ambient);
    return (vec_limit(vec_mul_v(light_color, scene->rec.albedo), color(1, 1, 1)));
}

t_vec reflect(t_vec light_dir, t_vec normal)
{
    t_vec ret;
    t_vec a;
    double n;

    //https://toymaker.tistory.com/entry/%EB%B0%98%EC%82%AC-%EB%B2%A1%ED%84%B0-Reflection-Vector
    n = vec_dot(light_dir, normal);
    //'광원을 향한 벡터'과 '충돌점의 법선벡터'를 내적하여 값을 구한뒤
    a = vec_mul(normal, n);
    //'법선벡터'에 값을 곱하여 '법선방향'과 '내적한 값의 크기'를 가지는 a벡터를 구함
    ret = vec_sum(vec_sum(vec_mul(light_dir, -1), a), a);
    //'광원을 향한 벡터'를 반전시키고 a를 두 번 더해주면 반사되어 나온 벡터를 구할 수 있음.
    return (ret);
}

t_color point_light_get(t_scene *scene, t_light *light)
{
    t_vec light_dir;
    //diffuse
    t_color diffuse;
    double kd;
    //specular
    t_vec specular;
    t_vec reflect_dir;
    double spec;
    double ksn; // shininess value of object
    double ks;  // specular strength
    double brightness;

    light_dir = vec_unit(vec_sub(light->orig, scene->rec.p));

    /*
    diffuse light
    '광원을 향한 벡터'과 '충돌점의 법선벡터'의 사이각를 이용해
    마주보고 있을수록 받는 빛의 양을 늘려줌.
    */
    kd = vec_dot(light_dir, scene->rec.normal); //단위벡터끼리 내적하면 0.0 ~ 1.0 사이의 값
    //내적을 이용하여 코사인0도에 가까울수록 서로 마주봄
    //90도를 넘어가면 음수이므로 빛의 영향을 전혀 받지 않음.
    if (kd < 0.0)
        kd = 0.0;
    diffuse = vec_mul(light->color, kd);

    /*
    specular light
    물체에 맞고 반사되어 우리 눈에 들어오는 빛을 추적함
    */
    reflect_dir = reflect(light_dir, scene->rec.normal);
    reflect_dir = vec_mul(reflect_dir, -1); //view_dir과 내적하기 위해 반전시킴
    spec = vec_dot(reflect_dir, scene->ray.dir);
    if (spec < 0.0)
        spec = 0.0;
    //내적하여 각도를 구함 -> diffuse와 마찬가지로 코사인0에 가까울수록 큰 값
    ksn = SHININESS; //물체의 매끈한(반짝거리는) 정도
    ks = SPECULAR_INTENSITY; //반사광의 세기 (광원의 밝기와는 다른 개념)
    spec = pow(spec, ksn);
    // 0.0~1.0 사이의 값인 spec을 몇번 제곱하는냐에 따라서 하이라이팅의 범위가 결정됨
    //https://oopy.lazyrockets.com/api/v2/notion/image?src=https%3A%2F%2Fcamo.githubusercontent.com%2Fba1ce58e05aed441aac19503958ed5b878d7a51d832b96f221b165ff4e11bddc%2F68747470733a2f2f6c6561726e6f70656e676c2e636f6d2f696d672f6c69676874696e672f62617369635f6c69676874696e675f73706563756c61725f7368696e696e6573732e706e67&blockId=9c804959-8c7b-4d77-8a69-3448849d33a9
    specular = vec_mul(vec_mul(light->color, ks), spec);
    // return (vec_sum(diffuse, specular));

    brightness = light->intensity * LUMEN; //광원의 밝기
    return (vec_mul(vec_sum(diffuse, specular), brightness));
}