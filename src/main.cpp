#include <iostream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

using namespace std;

static const bool HARDWARE_ACCELERATION = true;

int ScreenWidth = 720;
int ScreenHeight = 480;

int max_iteration = 200;

sf::Image pictures({ScreenWidth, ScreenHeight}, sf::Color::Black);
sf::Texture MapTexture(pictures);
sf::Sprite Background(MapTexture);

std::mutex renderMutex;
std::atomic<bool> renderReady = false;
std::atomic<bool> cancelRender = false;

std::thread renderThread;
std::atomic<bool> renderThreadRunning = false;
std::atomic<bool> newRenderRequested = false;
std::mutex paramMutex;
sf::Vector2<double> threadDelta;
sf::Vector2<double> threadOrigin;



sf::Color linearInterpolation(sf::Color& color1, sf::Color& color2, int iteration)
{

    double normalize = (double)iteration/(max_iteration/4);
    if(normalize>1) normalize = 1.0;

    // double normalize = (double)iteration/(max_iteration);


    return sf::Color(
               color1.r + normalize * (color2.r - color1.r),
               color1.g + normalize * (color2.g - color1.g),
               color1.b + normalize * (color2.b - color1.b)
           );
}

//sf::Image mandlebrot(long double xoffset, long double yoffset, long double zoom){

sf::Image mandlebrot(sf::Vector2<double> delta, sf::Vector2<double> origin){

    int iteration = 0;

    long double xtemp = 0.0;

    long double xVal = 0.0;
    long double yVal = 0.0;

    sf::Color color(255, 255, 255);
    sf::Color color1(40,40,150);
    sf::Color color2(200,60,60);

    sf::Image picture({ScreenWidth, ScreenHeight}, sf::Color::Black);

    // Calculate the aspect ratio
    double aspectRatio = double(ScreenWidth) / double(ScreenHeight);

    for(double x = 0; x<ScreenWidth; x++)
    {
        for(double y = 0; y<ScreenHeight; y++)
        {

            //long double scaledX = double((double(x)/double(ScreenWidth) - 0.5d) * 3.5d * zoom * aspectRatio/2.0d + xoffset);
            //long double scaledY = double((double(y)/double(ScreenHeight) - 0.5d) * 2.0d * zoom + yoffset);

            long double scaledX = double(x+0.5) * delta.x + origin.x;
            long double scaledY = double(y+0.5) * delta.y + origin.y;

            float breakValue = 0.0;
            iteration = 0;
            xVal = 0.0;
            yVal = 0.0;
            xtemp = 0.0;
            for(iteration = 0.0; iteration<max_iteration; iteration++){

                xtemp = xVal*xVal-yVal*yVal + scaledX;
                yVal = 2.0*xVal*yVal + scaledY;
                xVal = xtemp;
                breakValue = (xVal*xVal) + (yVal*yVal);

                if(breakValue >= 4.0){
                    break;
                }
            }

            color = linearInterpolation(color1,color2,iteration);
            picture.setPixel({x, y}, color);

            if(iteration >= max_iteration)
            {
            // this removes colour from the center
                color.r = 0;
                color.g = 0;
                color.b = 0;
                picture.setPixel({x, y}, color);

            }
        }
    }

    picture.flipVertically();
    //picture.saveToFile("result.png");
    return picture;
}



void cpuRenderThread(sf::Vector2<double> delta, sf::Vector2<double> origin) {

    renderReady = false;
    //sf::Image temp = mandlebrot(pan.x, pan.y, zoom);
    sf::Image temp = mandlebrot(delta, origin);

    if (cancelRender) return;

    std::lock_guard<std::mutex> lock(renderMutex);
    MapTexture.update(temp);
    renderReady = true;
    return;
}



void startRenderThread() {
    renderThreadRunning = true;
    renderThread = std::thread([] {
        while (renderThreadRunning) {
            if (newRenderRequested) {
                newRenderRequested = false;


                sf::Vector2<double> localDelta;
                sf::Vector2<double> localOrigin;
                {
                    std::lock_guard<std::mutex> lock(paramMutex);
                    localDelta = threadDelta;
                    localOrigin = threadOrigin;
                }

                sf::Image temp = mandlebrot(localDelta, localOrigin);
                {
                    std::lock_guard<std::mutex> lock(renderMutex);
                    MapTexture.update(temp);
                    renderReady = true;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}




int main()
{
    bool next = true;

    bool isDragging = false;
    sf::Vector2i dragStartMouse;
    sf::Vector2<double> dragStartCenter;

    sf::Vector2<double> pan = { -0.5, 0.0 };

    //long double panX = -0.5;
    //long double panY = 0.0;

    double basePanSpeed = 0.01f;
    double panStep = 0.0;

    long double zoom = 0.9;
    long double baseZoom = 0.9;

    int zoomSteps = 1;

    sf::RenderWindow window(sf::VideoMode({ScreenWidth, ScreenHeight}), "mandlebrot");
    window.setFramerateLimit(60);

    // Create a fullscreen rectangle
    sf::RectangleShape fullscreenQuad(sf::Vector2f(window.getSize().x, window.getSize().y));

    sf::Shader shader;
    if (!shader.loadFromFile("src/mandlebrot.frag", sf::Shader::Type::Fragment))
        return -1;

    shader.setUniform("resolution", sf::Glsl::Vec2(window.getSize().x, window.getSize().y));

    shader.setUniform("zoom",float(zoom));

    sf::Clock clock;

    shader.setUniform("pan",sf::Glsl::Vec2(0.0f, 0.0f));

    //pictures = mandlebrot(pan.x,pan.y,zoom);
    //MapTexture.update(pictures);

    window.setActive(false);
    //sf::Thread thread(&renderingThread, &window);
    //thread.launch();

     if(HARDWARE_ACCELERATION){
       // std::thread(cpuRenderThread, pan, zoom).detach();
       startRenderThread();
     }

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {

            if (event->is<sf::Event::Closed>())
            {
                return 0;
            }
            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                fullscreenQuad.setSize(sf::Vector2f(resized->size.x, resized->size.y));
                shader.setUniform("resolution", sf::Glsl::Vec2(resized->size.x, resized->size.y));

               ScreenWidth = resized->size.x;
               ScreenHeight = resized->size.y;

                next = true;
            }




            if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()){
                switch (mouseWheelScrolled->wheel){
                    case sf::Mouse::Wheel::Vertical:
                       // std::cout << "wheel type: vertical" << std::endl;

                        if(mouseWheelScrolled->delta >0){

                            zoomSteps++;
                            next = true;

                        }else{
                            zoomSteps--;
                            next = true;
                        }
                        break;
                    case sf::Mouse::Wheel::Horizontal:
                        //std::cout << "wheel type: horizontal" << std::endl;
                        break;
                }
                //std::cout << "wheel movement: " << mouseWheelScrolled->delta << std::endl;
                //std::cout << "mouse x: " << mouseWheelScrolled->position.x << std::endl;
                //std::cout << "mouse y: " << mouseWheelScrolled->position.y << std::endl;
            }


            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mouseButtonPressed->button == sf::Mouse::Button::Left)
                {
                    //std::cout << "the left button was pressed" << std::endl;
                    //std::cout << "mouse x: " << mouseButtonPressed->position.x << std::endl;
                    //std::cout << "mouse y: " << mouseButtonPressed->position.y << std::endl;

                    isDragging = true;
                    dragStartMouse = sf::Mouse::getPosition(window);
                    dragStartCenter = pan; // the center when you first clicked

                }
            }


               if (const auto* MouseButtonReleased = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (MouseButtonReleased->button == sf::Mouse::Button::Left)
                {
                    //std::cout << "the left button was released" << std::endl;
                    //std::cout << "mouse x: " << MouseButtonReleased->position.x << std::endl;
                    //std::cout << "mouse y: " << MouseButtonReleased->position.y << std::endl;
                    isDragging = false;
                }
            }
        }





        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add))
        {
            zoomSteps++;
            next = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract))
        {
            zoomSteps--;
            next = true;
        }


        zoom = pow(baseZoom, zoomSteps);
        panStep = basePanSpeed * zoom;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            pan.x -= panStep;
            next = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            pan.x += panStep;
            next = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            pan.y += panStep;
            next = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            pan.y -= panStep;
            next = true;
        }


        if (isDragging) {
            sf::Vector2i currentMouse = sf::Mouse::getPosition(window);
            sf::Vector2i mouseDelta = dragStartMouse - currentMouse; // reversed direction (drag moves view)


            double aspectRatio = double(window.getSize().x) / double(window.getSize().y);
            double scaledZoomX = zoom * aspectRatio;
            double scaledZoomY = zoom;

            sf::Vector2<double> pixelDelta = {
                scaledZoomX / double(window.getSize().x),
                scaledZoomY / double(window.getSize().y)
            };

            double temp = -1;

            // Convert pixel delta to complex space delta
            sf::Vector2<double> complexDelta = {
                mouseDelta.x * pixelDelta.x,
                mouseDelta.y * temp * pixelDelta.y
            };

            pan = dragStartCenter + complexDelta;
            next = true;
        }


        double aspectRatio = double(ScreenWidth) / double(ScreenHeight);

        double scaledZoom = zoom * aspectRatio/2.0d;
/*
        sf::Vector2<double> origin = { 3.5d * scaledZoom / double(ScreenWidth), 2.0d * zoom / double(ScreenHeight) };

        sf::Vector2<double> delta = {
            (pan.x-0.5d * 3.5d * scaledZoom),
            (pan.y-0.5d * 2.0d * zoom)
        };
*/


        sf::Vector2<double> delta = {
            zoom * aspectRatio / double(window.getSize().x),
            zoom / double(window.getSize().y)
        };

        sf::Vector2<double> origin = {
            pan.x - delta.x * double(window.getSize().x) * 0.5,
            pan.y - delta.y * double(window.getSize().y) * 0.5
        };

        if(next){
            if(!HARDWARE_ACCELERATION){
                pictures = mandlebrot(delta,origin);
                MapTexture.update(pictures);
            }else{
                //cancelRender = true;
                //std::this_thread::sleep_for(std::chrono::milliseconds(10));
                //cancelRender = false;
                //std::thread(cpuRenderThread, delta, origin).detach();
                {
                    std::lock_guard<std::mutex> lock(paramMutex);
                    threadDelta = delta;
                    threadOrigin = origin;
                }
                renderReady = false;
                newRenderRequested = true;
           }
            next = false;
        }


        shader.setUniform("origin", sf::Glsl::Vec2(origin.x, origin.y));
        shader.setUniform("delta", sf::Glsl::Vec2(delta.x, delta.y));

        shader.setUniform("zoom",float(zoom));
        shader.setUniform("pan",sf::Glsl::Vec2(float(pan.x), float(pan.y)));

        window.clear();

        if(HARDWARE_ACCELERATION){
            //window.draw(fullscreenQuad, &shader);
            if(zoom < 5e-5){
                std::lock_guard<std::mutex> lock(renderMutex);
                window.draw(Background);
                cout<<"cpu Deep ";
            }else{
             if (renderReady) {
                std::lock_guard<std::mutex> lock(renderMutex);
                window.draw(Background);
                cout<<"cpu ";
            } else {
                window.draw(fullscreenQuad, &shader);
                cout<<"shader ";
            }

            }

        }else{
            std::lock_guard<std::mutex> lock(renderMutex);
            window.draw(Background);
        }
        window.display();
    }
    renderThreadRunning = false;
    if (renderThread.joinable()) renderThread.join();
    return 0;
}
