#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <chrono>

// ----------------------------------------------------
// ЛОГИ И ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ----------------------------------------------------
static inline void log(const std::string& msg) {
    std::cout << "[LOG]: " << msg << std::endl;
}

// Простой таймер для измерения времени на рендер
class ScopedTimer {
public:
    ScopedTimer(const std::string& message)
        : msg_(message), start_(std::chrono::high_resolution_clock::now()) {
    }
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
        std::cout << "[TIMER] " << msg_ << ": " << duration << " ms" << std::endl;
    }
private:
    std::string msg_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// ----------------------------------------------------
// ВЕКТОР И ЛУЧИ
// ----------------------------------------------------
struct Vec3 {
    float x, y, z;
    
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float f)       const { return Vec3(x * f, y * f, z * f); }
    Vec3 operator/(float f)       const { return Vec3(x / f, y / f, z / f); }
    
    float dot(const Vec3& v)  const { return x * v.x + y * v.y + z * v.z; }
    float length()            const { return std::sqrt(dot(*this)); }
    
    Vec3 normalize() const {
        float len = length();
        return (len < 1e-9f) ? Vec3(0,0,0) : *this / len;
    }
    
    // Покомпонентное умножение (например, для смешивания цветов)
    Vec3 multiply(const Vec3& v) const {
        return Vec3(x * v.x, y * v.y, z * v.z);
    }
};

struct Ray {
    Vec3 origin;
    Vec3 direction;
    
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d.normalize()) {}
};

// ----------------------------------------------------
// ПЕРЕСЕЧЕНИЯ
// ----------------------------------------------------
struct Hit {
    float t;           // Расстояние до точки пересечения
    Vec3 point;        // Точка пересечения
    Vec3 normal;       // Нормаль в точке пересечения
    float density;     // Плотность среды в точке пересечения
    
    Hit() : t(INFINITY), density(0.0f) {}
};

// ----------------------------------------------------
// БАЗОВЫЙ КЛАСС ОБЪЕКТА
// ----------------------------------------------------
class Object {
public:
    Vec3 color;  // RGB цвет объекта
    
    Object(const Vec3& c = Vec3(1.0f, 1.0f, 1.0f)) : color(c) {}
    virtual bool intersect(const Ray& ray, Hit& hit) const = 0;
    virtual float getDensity(const Vec3& point) const = 0;
    virtual Vec3 getColor(const Vec3& /*point*/) const { return color; }
    virtual ~Object() = default;
};

// ----------------------------------------------------
// СФЕРА
// ----------------------------------------------------
class Sphere : public Object {
    Vec3 center;
    float radius;
    float volumeDensity;

public:
    Sphere(const Vec3& c, float r, float d = 0.1f, const Vec3& col = Vec3(1.0f, 1.0f, 1.0f)) 
        : Object(col), center(c), radius(r), volumeDensity(d) {}
    
    bool intersect(const Ray& ray, Hit& hit) const override {
        Vec3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) return false;
        
        float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
        if (t < 0) {
            t = (-b + std::sqrt(discriminant)) / (2.0f * a);
            if (t < 0) return false;
        }
        
        hit.t = t;
        hit.point = ray.origin + ray.direction * t;
        hit.normal = (hit.point - center).normalize();
        hit.density = volumeDensity;
        
        return true;
    }
    
    float getDensity(const Vec3& point) const override {
        float dist = (point - center).length();
        if (dist > radius) return 0.0f;
        // Простая линейная модель распределения плотности внутри сферы
        return volumeDensity * (1.0f - dist / radius);
    }
    
    Vec3 getCenter()    const { return center; }
    float getRadius()   const { return radius; }
    float getDensityValue() const { return volumeDensity; }
    
    void setColor(const Vec3& newColor) { color = newColor; }
};

// ----------------------------------------------------
// ПЛОСКОСТЬ
// ----------------------------------------------------
class Plane : public Object {
    Vec3 normal;
    float distance;
    float volumeDensity;

public:
    Plane(const Vec3& n, float d, float vd = 0.0f, const Vec3& col = Vec3(1.0f, 1.0f, 1.0f)) 
        : Object(col), normal(n.normalize()), distance(d), volumeDensity(vd) {}
    
    bool intersect(const Ray& ray, Hit& hit) const override {
        float denom = normal.dot(ray.direction);
        if (std::abs(denom) < 1e-6) return false;
        
        float t = -(normal.dot(ray.origin) + distance) / denom;
        if (t < 0) return false;
        
        hit.t = t;
        hit.point = ray.origin + ray.direction * t;
        hit.normal = normal;
        hit.density = volumeDensity;
        return true;
    }
    
    float getDensity(const Vec3& /*point*/) const override {
        // Плоскость во многих задачах предполагается бесконечно тонкой;
        // но если хотим “объём”, можно сделать так, как в сфере
        return volumeDensity;
    }
    
    Vec3 getNormal()       const { return normal; }
    float getDistance()    const { return distance; }
    float getDensityValue() const { return volumeDensity; }
    
    void setColor(const Vec3& newColor) { color = newColor; }
};

// ----------------------------------------------------
// СЦЕНА
// ----------------------------------------------------
class Scene {
    std::vector<Object*> objects;
    Vec3 lightPos;
    float lightIntensity;

public:
    Scene(const Vec3& light_pos, float intensity) 
        : lightPos(light_pos), lightIntensity(intensity) {}
    
    ~Scene() {
        for (auto obj : objects) delete obj;
    }
    
    void addObject(Object* obj) {
        objects.push_back(obj);
    }
    
    // Основная функция для “объёмного” света
    float calculateVolumetricLight(const Ray& ray, float maxDist, Vec3& outColor, int numSamples = 15) const {
        // Чем больше numSamples, тем лучше качество, но медленнее рендер
        float stepSize = maxDist / numSamples;
        float totalLight = 0.0f;
        Vec3 accumulatedColor(0.0f, 0.0f, 0.0f);
        float transmittance = 1.0f;  // Коэффициент пропускания (эксп. затухание)
        
        for (int i = 0; i < numSamples; ++i) {
            Vec3 samplePoint = ray.origin + ray.direction * (i * stepSize);
            float density = 0.0f;
            Vec3 sampleColor(0.0f, 0.0f, 0.0f);
            
            // Суммируем плотность и цвет от всех объектов
            for (const auto& obj : objects) {
                float objDensity = obj->getDensity(samplePoint);
                density += objDensity;
                if (objDensity > 0) {
                    // При большом кол-ве объектов можно усреднять цвет
                    sampleColor = sampleColor + obj->getColor(samplePoint) * objDensity;
                }
            }
            
            if (density > 0) {
                // Усреднённый цвет для данной точки
                sampleColor = sampleColor / density;
                
                // Направление к источнику света
                Vec3 toLight = lightPos - samplePoint;
                float distToLight = toLight.length();
                toLight = toLight.normalize();
                
                // Интенсивность, убывающая по квадрату расстояния
                float lightContribution = lightIntensity / (distToLight * distToLight);
                
                // Учитываем затухание света при прохождении через среду
                float contribution = density * lightContribution * stepSize * transmittance;
                totalLight += contribution;
                
                // Накопленный цвет
                accumulatedColor = accumulatedColor + sampleColor * contribution;
                
                // Экспоненциальное затухание: чем больше плотность, тем сильнее падает transmittance
                transmittance *= std::exp(-density * stepSize);
            }
        }
        
        outColor = (totalLight > 1e-9f) ? (accumulatedColor / totalLight) : Vec3(0, 0, 0);
        return totalLight;
    }
    
    // Изменение плотности
    void adjustDensity(int objectIndex, float deltaDensity) {
        if (objectIndex < 0 || objectIndex >= (int)objects.size()) {
            log("Wrong object index for adjustDensity!");
            return;
        }
        
        if (auto sphere = dynamic_cast<Sphere*>(objects[objectIndex])) {
            float oldDensity = sphere->getDensityValue();
            float newDensity = std::max(0.0f, oldDensity + deltaDensity); // не даём стать отрицательным
            log("Changing sphere density from " + std::to_string(oldDensity) + " to " + std::to_string(newDensity));
            
            // Пересоздадим объект с новой плотностью (можно было сделать setter)
            auto newSphere = new Sphere(sphere->getCenter(), sphere->getRadius(), newDensity, sphere->color);
            
            delete objects[objectIndex];
            objects[objectIndex] = newSphere;
        }
        else if (auto plane = dynamic_cast<Plane*>(objects[objectIndex])) {
            float oldDensity = plane->getDensityValue();
            float newDensity = std::max(0.0f, oldDensity + deltaDensity);
            log("Changing plane density from " + std::to_string(oldDensity) + " to " + std::to_string(newDensity));
            
            auto newPlane = new Plane(plane->getNormal(), plane->getDistance(), newDensity, plane->color);
            
            delete objects[objectIndex];
            objects[objectIndex] = newPlane;
        }
    }
    
    // Изменение цвета
    void adjustColor(int objectIndex, const Vec3& colorDelta) {
        if (objectIndex < 0 || objectIndex >= (int)objects.size()) {
            log("Wrong object index for adjustColor!");
            return;
        }
        
        if (auto sphere = dynamic_cast<Sphere*>(objects[objectIndex])) {
            Vec3 oldColor = sphere->color;
            Vec3 newColor = oldColor + colorDelta;
            // Ограничим значения [0,1]
            newColor.x = std::max(0.0f, std::min(1.0f, newColor.x));
            newColor.y = std::max(0.0f, std::min(1.0f, newColor.y));
            newColor.z = std::max(0.0f, std::min(1.0f, newColor.z));
            
            log("Changing sphere color from (" + std::to_string(oldColor.x) + ","
                                              + std::to_string(oldColor.y) + ","
                                              + std::to_string(oldColor.z) + ") to ("
                                              + std::to_string(newColor.x) + ","
                                              + std::to_string(newColor.y) + ","
                                              + std::to_string(newColor.z) + ")");
            sphere->setColor(newColor);
        }
        else if (auto plane = dynamic_cast<Plane*>(objects[objectIndex])) {
            Vec3 oldColor = plane->color;
            Vec3 newColor = oldColor + colorDelta;
            newColor.x = std::max(0.0f, std::min(1.0f, newColor.x));
            newColor.y = std::max(0.0f, std::min(1.0f, newColor.y));
            newColor.z = std::max(0.0f, std::min(1.0f, newColor.z));
            
            log("Changing plane color from (" + std::to_string(oldColor.x) + ","
                                              + std::to_string(oldColor.y) + ","
                                              + std::to_string(oldColor.z) + ") to ("
                                              + std::to_string(newColor.x) + ","
                                              + std::to_string(newColor.y) + ","
                                              + std::to_string(newColor.z) + ")");
            plane->setColor(newColor);
        }
    }
};

// ----------------------------------------------------
// ОСНОВНАЯ ФУНКЦИЯ
// ----------------------------------------------------
int main() {
    log("Starting application...");

    const int WIDTH  = 800;
    const int HEIGHT = 600;
    
    // Создаём окно
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Ray Tracing + Volumetric Light (No OpenMP)");
    window.setFramerateLimit(30); // ограничим FPS до 30 для стабильности
    
    // Подготовим объекты SFML для вывода
    sf::Image   image;
    sf::Texture texture;
    sf::Sprite  sprite;
    image.create(WIDTH, HEIGHT);
    
    log("Created SFML window with size: " + std::to_string(WIDTH) + "x" + std::to_string(HEIGHT));
    
    // Создаём сцену с двумя сферами и одной плоскостью (точно по заданию)
    Scene scene(Vec3(5.0f, 5.0f, 5.0f), 50.0f);

    // Две сферы (как требуется в задании)
    scene.addObject(new Sphere(Vec3(-1.5f, 0.0f, 5.0f), 1.0f, 0.1f, Vec3(1.0f, 0.2f, 0.2f))); // первая сфера
    scene.addObject(new Sphere(Vec3(1.5f, 0.0f, 5.0f), 1.0f, 0.1f, Vec3(0.2f, 1.0f, 0.2f)));  // вторая сфера

    // Одна плоскость (как требуется в задании)
    scene.addObject(new Plane(Vec3(0.0f, 1.0f, 0.0f), 2.0f, 0.02f, Vec3(0.5f, 0.5f, 1.0f))); // плоскость

    log("Scene created: 2 spheres, 1 plane (as per requirements).");
    
    // Позиция камеры
    Vec3 cameraPos(0.0f, 0.0f, -5.0f);
    
    // ------------------------------------------------
    // Функция для рендеринга
    // ------------------------------------------------
    auto renderScene = [&](int numSamples = 15) {
        ScopedTimer timer("Render Scene");  // автоматический вывод времени
        log("Rendering scene... (numSamples=" + std::to_string(numSamples) + ")");
        
        // Полный проход по каждому пикселю — однопоточный
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                float u = (2.0f * x - WIDTH) / static_cast<float>(HEIGHT);
                float v = (2.0f * y - HEIGHT) / static_cast<float>(HEIGHT);
                
                Ray ray(cameraPos, Vec3(u, v, 1.0f).normalize());
                Vec3 pixelColor;
                
                // Делать трассировку с объёмными эффектами
                scene.calculateVolumetricLight(ray, 20.0f, pixelColor, numSamples);
                
                // Преобразуем цвет в диапазон [0..255]
                uint8_t r = static_cast<uint8_t>(std::min(255.0f, pixelColor.x * 255.0f));
                uint8_t g = static_cast<uint8_t>(std::min(255.0f, pixelColor.y * 255.0f));
                uint8_t b = static_cast<uint8_t>(std::min(255.0f, pixelColor.z * 255.0f));
                
                // Записываем в картинку
                image.setPixel(x, y, sf::Color(r, g, b));
            }
        }
        texture.loadFromImage(image);
        sprite.setTexture(texture);
        
        log("Scene render complete.");
    };
    
    // Первый рендер (полноценный)
    renderScene(15);
    
    // ------------------------------------------------
    // ОСНОВНОЙ ЦИКЛ
    // ------------------------------------------------
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed 
             || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                log("Closing application...");
                window.close();
                break;
            }
            
            if (event.type == sf::Event::KeyPressed) {
                bool needsUpdate = false;
                float densityDelta = 0.05f;
                
                // Управление плотностью только для двух сфер и плоскости
                if (event.key.code == sf::Keyboard::Num1) {
                    scene.adjustDensity(0, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? -densityDelta : densityDelta);
                    needsUpdate = true;
                }
                else if (event.key.code == sf::Keyboard::Num2) {
                    scene.adjustDensity(1, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? -densityDelta : densityDelta);
                    needsUpdate = true;
                }
                else if (event.key.code == sf::Keyboard::Num3) {
                    scene.adjustDensity(2, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? -densityDelta : densityDelta);
                    needsUpdate = true;
                }
                
                // Управление цветом
                float colorStep = 0.1f;
                if (event.key.code == sf::Keyboard::R || 
                    event.key.code == sf::Keyboard::G || 
                    event.key.code == sf::Keyboard::B) {
                    
                    int objectIndex = -1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) objectIndex = 0;
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) objectIndex = 1;
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) objectIndex = 2;
                    
                    if (objectIndex != -1) {
                        Vec3 colorDelta(0, 0, 0);
                        if (event.key.code == sf::Keyboard::R) 
                            colorDelta.x = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? -colorStep : colorStep;
                        else if (event.key.code == sf::Keyboard::G) 
                            colorDelta.y = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? -colorStep : colorStep;
                        else if (event.key.code == sf::Keyboard::B) 
                            colorDelta.z = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? -colorStep : colorStep;
                        
                        scene.adjustColor(objectIndex, colorDelta);
                        needsUpdate = true;
                    }
                }
                
                if (needsUpdate) {
                    log("Changes detected, rendering preview...");
                    renderScene(5); // Быстрый предпросмотр
                }
            }
            
            // Полный рендер при отпускании клавиши
            if (event.type == sf::Event::KeyReleased && 
                (event.key.code == sf::Keyboard::Num1 || 
                 event.key.code == sf::Keyboard::Num2 || 
                 event.key.code == sf::Keyboard::Num3 || 
                 event.key.code == sf::Keyboard::R || 
                 event.key.code == sf::Keyboard::G || 
                 event.key.code == sf::Keyboard::B)) {
                
                log("Performing full quality render...");
                renderScene(15); // Полное качество
            }
        }
        
        // Рисуем результат
        window.clear();
        window.draw(sprite);
        window.display();
    }
    
    return 0;
}