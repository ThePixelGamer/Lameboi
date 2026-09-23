#version 330 core

#define Epsilon 0.000001

uniform sampler3D volume;
uniform float voxel_size;

uniform vec3 colorPalette[256];

out vec4 FragColor;

flat in vec3 model_cam_pos;
in vec3 fV;
in vec3 pos;

struct hit_t {
    float depth;
    vec3 voxel_pos;
    vec3 pixel_pos;
    vec3 uvw;
    vec2 uv;
    vec3 normal;
    int material;
};

vec3 uvw_to_normal(vec3 uvw){
    // find normal
    vec3 hit_to_center = uvw - vec3(0.5);
    vec3 abs_hit_to_center = abs(hit_to_center);
    float max_component = max(max(abs_hit_to_center.x, abs_hit_to_center.y), abs_hit_to_center.z);

    vec3 normal;
    if (abs_hit_to_center.x == max_component) {
        normal = vec3(sign(hit_to_center.x), 0.0, 0.0);
    } else if (abs_hit_to_center.y == max_component) {
        normal = vec3(0.0, sign(hit_to_center.y), 0.0);
    } else {
        normal = vec3(0.0, 0.0, sign(hit_to_center.z));
    }
    return normal;
}

vec2 uvw_to_uv(vec3 uvw){
    if (uvw.x == 0.0 && uvw.y == 0.0 && uvw.z == 0.0) return vec2(0.0, 0.0);
    if (uvw.x == 1.0 && uvw.y == 1.0 && uvw.z == 1.0) return vec2(1.0, 1.0);
    if (uvw.x == 0.0 && uvw.y == 1.0 && uvw.z == 1.0) return vec2(0.0, 1.0);
    if (uvw.x == 1.0 && uvw.y == 0.0 && uvw.z == 1.0) return vec2(1.0, 0.0);
    if (uvw.x == 1.0 && uvw.y == 1.0 && uvw.z == 0.0) return vec2(1.0, 0.0);
    if (uvw.x == 0.0 && uvw.y == 0.0 && uvw.z == 1.0) return vec2(0.0, 1.0);
    vec3 tangents = vec3(1) - abs(uvw_to_normal(uvw));
    vec3 a, b;
    if (tangents.x == 0.0f) {
        a = vec3(0, 1, 0);
        b = vec3(0, 0, 1);
    } else if (tangents.y == 0.0f) {
        a = vec3(1, 0, 0);
        b = vec3(0, 0, 1);
    } else {
        a = vec3(1, 0, 0);
        b = vec3(0, 1, 0);
    }
    return vec2(dot(b, uvw), dot(a, uvw));
}

void findStartPos(out vec3 ro, out vec3 normal) {
    vec3 dir = normalize(fV);
    // x/0 is undefined behaviour
    //if(dir.x == 0.0) dir.x = 0.0000001;
    //if(dir.y == 0.0) dir.y = 0.0000001;
    //if(dir.z == 0.0) dir.z = 0.0000001;
    vec3 dir_inv = vec3(1.0)/dir;
    // move origin up to before intersecting the box
    vec3 origin = pos - 2.0 * dir;
    float t1 = (0.0 - origin.x) * dir_inv.x;
    float t2 = (1.0 - origin.x) * dir_inv.x;
    float t3 = (0.0 - origin.y) * dir_inv.y;
    float t4 = (1.0 - origin.y) * dir_inv.y;
    float t5 = (0.0 - origin.z) * dir_inv.z;
    float t6 = (1.0 - origin.z) * dir_inv.z;

    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    // we are sure that we hit the box otherwise we would not render here!
    // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    //if (tmax < 0) return { true};
    // if tmin > tmax, ray doesn't intersect AABB
    //if (tmin > tmax) return { false};


    // first intersection with cube
    vec3 near =  origin + dir * (tmin + Epsilon * 10.0f);
    vec3 far =  origin + dir * (tmax - Epsilon);
    // if camera is closer to the pos on the backface than the intersect, return the camera pos
    if (length(near - pos) > length(model_cam_pos - pos)){
        ro = model_cam_pos;
		normal = vec3(0, 0, 0);// 0.0 - 1.0
    }
	else {
		ro = near;
		normal = uvw_to_normal(near);
	}
}

float isInside(vec3 pos){
    // this works but did not notice performance difference
    /*
        return step(0.0, pos.x)*step(pos.x, 1.0) *
        step(0.0, pos.y)*step(pos.y, 1.0) *
        step(0.0, pos.z)*step(pos.z, 1.0);
    */
    if (pos.x < 0.0 || pos.x > 1.0) return 0.0;
    if (pos.y < 0.0 || pos.y > 1.0) return 0.0;
    if (pos.z < 0.0 || pos.z > 1.0) return 0.0;
    return 1.0;
}

hit_t fvta_step(){
    vec3 ro, normal;
	findStartPos(ro, normal);
    vec3 rd = normalize(fV);
    float voxel_size_local = voxel_size*1; //1 - normal, 2 - twice the size of the steps
    vec3 voxel_index = floor(ro * (1./voxel_size_local));
    vec3 voxel_pos = voxel_size_local * voxel_index;
    vec3 rs = sign(rd);
    vec3 deltaDist = voxel_size_local/rd;
    vec3 sideDist = ((voxel_pos-ro)/voxel_size_local + 0.5 + rs * 0.5) * deltaDist;
    int max_steps = int(3.0/voxel_size_local);

    vec3 final_pos = ro * 1./voxel_size_local;
    float t = 0.0;
    vec3 pos = ro;//voxel_size_local * (voxel_index + 0.1);
    vec3 uvw = (pos - voxel_pos)/voxel_size_local;
    vec2 uv = uvw_to_uv(uvw);

    for (int i = 0; i < max_steps; i++){
        if (isInside(pos) < 0.5) {
            //return hit_t(t, voxel_pos, pos, uvw, uv, vec3(1,0,0), 0);
            discard;
        }
        int mat = int(round(texture(volume, voxel_pos + 0.1 * voxel_size).r*255));
        if (mat > 0){
            return hit_t(t, voxel_pos, pos, uvw, uv, normal, mat);
        }

        // black magic compare between sideDist.x < sideDist.y && sideDist.x < sideDist.z etc
        vec3 mm = step(sideDist.xyz, sideDist.yxy) * step(sideDist.xyz, sideDist.zzx);
        normal = -mm * rs;

        voxel_pos += voxel_size_local * -normal;

        // other stuff that is nice to know
        vec3 mini = ((voxel_pos-ro)/voxel_size_local + 0.5 - 0.5*vec3(rs))*deltaDist;
        t = max (mini.x, max (mini.y, mini.z));
        pos = ro + rd * (t + Epsilon * 2.0);
        uvw = (pos - voxel_pos)/voxel_size_local;
        uv = vec2(dot(mm.yzx, uvw), dot(mm.zxy, uvw));

        sideDist += -normal * deltaDist;
    }
    discard;
}

void main() {
    hit_t hit = fvta_step();
    if (isInside(hit.pixel_pos - 0.01 * hit.normal * voxel_size) < 0.5){
        discard;
    }

    FragColor = vec4(colorPalette[hit.material], 1.0f);
    //FragColor = vec4(pos, 1.0f);
} 
