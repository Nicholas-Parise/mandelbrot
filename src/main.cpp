#include <iostream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <vector>

using namespace std;

static const bool HARDWARE_ACCELERATION = true;

int ScreenWidth = 720;
int ScreenHeight = 480;

int max_iteration = 200;

sf::Image pictures({ScreenWidth, ScreenHeight}, sf::Color::Black);
sf::Texture MapTexture(pictures);
sf::Sprite Background(MapTexture);


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

sf::Image mandlebrot(long double xoffset, long double yoffset, long double zoom){
    int iteration = 0;

    long double xtemp = 0;

    long double xVal = 0;
    long double yVal = 0;

    sf::Color color(255, 255, 255);
    sf::Color color1(40,40,150);
    sf::Color color2(200,60,60);

    sf::Image picture({ScreenWidth, ScreenHeight}, sf::Color::Black);

    for(int x = 1; x<ScreenWidth-1; x++)
    {
        for(int y = 1; y<ScreenHeight-1; y++)
        {

            long double scaledX = double((double(x)/double(ScreenWidth) - 0.5d) * 3.5d * zoom + xoffset);
            long double scaledY = double((double(y)/double(ScreenHeight) - 0.5d) * 2.0d * zoom + yoffset);

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

    //picture.saveToFile("result.png");
    return picture;
}



void renderingThread(sf::RenderWindow* window)
{
    window->setActive(true);
    window->setFramerateLimit(1);
    // the rendering loop
    while (window->isOpen())
    {
        // draw...
        window->clear();



        window->draw(Background);


        sf::View view = window->getView();
        window->display();
    }
}


int main()
{

    bool next = false;

    long double panX = -0.5;
    long double panY = 0.0;
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

    pictures = mandlebrot(panX,panY,zoom);
    MapTexture.update(pictures);

    window.setActive(false);
    //sf::Thread thread(&renderingThread, &window);
    //thread.launch();

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
            panX -= panStep;
            next = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            panX += panStep;
            next = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            if(HARDWARE_ACCELERATION){
                panY += panStep;
            }else{
                panY -= panStep;
            }
            next = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            if(HARDWARE_ACCELERATION){
                panY -= panStep;
            }else{
                panY += panStep;
            }
            next = true;
        }


        if(next){
            if(!HARDWARE_ACCELERATION){
                pictures = mandlebrot(panX,panY,zoom);
                MapTexture.update(pictures);
            }

            next = false;
            // cout<<panX<<" "<<panY<<" "<<zoom<<endl;
        }

        shader.setUniform("zoom",float(zoom));
        shader.setUniform("pan",sf::Glsl::Vec2(float(panX), float(panY)));

        window.clear();

        if(HARDWARE_ACCELERATION){
            window.draw(fullscreenQuad, &shader);
        }else{
            window.draw(Background);
        }
        window.display();
    }
    return 0;
}
