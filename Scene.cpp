
#include "Scene.hpp"
#include "Vector.hpp"
#include "Material.hpp"
#include <cmath>


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{


    Vector3f L_dir, L_indir;
    Intersection lit;
    float pdf_lit;
    sampleLight(lit, pdf_lit);
    Vector3f pos = ray.origin, x = lit.coords;
    Vector3f  ws = normalize(x - pos);
    Ray ray_lit = Ray(pos, ws);
    Intersection isect_q = intersect(ray_lit);
    if((isect_q.coords - x).norm() < eps)
    {
        L_dir = std::exp(-kappa * (x - pos).norm() * 0.0001) * lit.emit * dotProduct(-ws, normalize(lit.normal))/ (dotProduct(pos - x, pos - x) * (pdf_lit + 1e-6));
    }
    
    

    float s, u;
    s = 1.0 * rand() / RAND_MAX;//(0, inf)
    u = 1.0 * rand() / RAND_MAX * kappa;//(0, kappa)
    while(kappa * s > std::log(kappa) - std::log(u + eps))
    {
        s = 1.0 * rand() / RAND_MAX;//(0, inf)
        u = 1.0 * rand() * kappa / RAND_MAX;//(0, kappa)
    }
    Intersection isect_p = intersect(ray);
    if(isect_p.distance <= s * 10000)
    {
        //Onto surface
        Material *m = isect_p.m;
        if( m->hasEmission()) L_indir = m->getEmission();
        else
        {
            Vector3f p = isect_p.coords, n = normalize(isect_p.normal);
            Vector3f wo = normalize(-ray.direction);

            float rd = 1.0 * rand() / (RAND_MAX);
            if(rd < RussianRoulette)
            {
                Vector3f wi = normalize(m->sample(wo, n));
                Ray rlit = Ray(p, wi);
                Intersection isect_r = intersect(rlit);
                if(isect_r.happened && !isect_r.m->hasEmission())
                    L_indir = castRay(rlit, depth + 1) * m->eval(wo, wi, n) * dotProduct(wi, n) / ((m->pdf(wo, wi, n) + 1e-6) * RussianRoulette * kappa);
            }
        }

    }
    else
    {
        //Into media
        float rd;
        rd = 1.0 * rand() / RAND_MAX;
        if(rd < 0.9)//scatter or absorb
        {
            float rd1;
            float theta, alpha;
            theta = 1.0 * rand() * M_PI / RAND_MAX;//(0, pi)
            rd1 = 5.0 * rand() / RAND_MAX;//(0.5, 5)
            while(rd1 > 0.5 + 4.5 * std::pow((1 + cos(theta)) * 0.5, 8))
            {
                theta = 1.0 * rand() * M_PI / RAND_MAX;
                rd1 = 5.0 * rand() / RAND_MAX;
            }
            alpha = 2.0 * rand() * M_PI / RAND_MAX;//(0, 2 * pi)
            Vector3f dir_new = rotate(ray.direction, theta, alpha);
            L_indir = sigma * castRay(Ray(ray(s * 10000), dir_new), depth + 1) / kappa;
        }
    }
    return L_dir + L_indir;
}