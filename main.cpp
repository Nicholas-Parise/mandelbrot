#include <iostream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <vector>

using namespace std;

int ScreenWidth = 720;
int ScreenHeight = 480;

int max_iteration = 200;

sf::View view(sf::FloatRect({0.f, 0.f}, {ScreenWidth, ScreenHeight}));
sf::Image pictures({ScreenWidth, ScreenHeight}, sf::Color::Black);
sf::Texture MapTexture(pictures);
sf::Sprite Background(MapTexture);


sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight)
{

    // Compares the aspect ratio of the window to the aspect ratio of the view,
    // and sets the view's viewport accordingly in order to archieve a letterbox effect.
    // A new view (with a new viewport set) is returned.

    float windowRatio = windowWidth / (float) windowHeight;
    float viewRatio = view.getSize().x / (float) view.getSize().y;
    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = true;
    if (windowRatio < viewRatio)
        horizontalSpacing = false;

    // If horizontalSpacing is true, the black bars will appear on the left and right side.
    // Otherwise, the black bars will appear on the top and bottom.

    if (horizontalSpacing)
    {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    }

    else
    {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport( sf::FloatRect({posX, posY}, {sizeX, sizeY}) );

    return view;
}


long double scaleRange(long double Min_Range, long double Max_Range, long double Min_Num, long double Max_num, long double Num)
{

//this turns a number from (0 -> 1920) to (-2.5 -> 1)

    return ((((Max_num-Min_Num)*(Num-Min_Range)) / (Max_Range-Min_Range))+Min_Num);
}


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


sf::Image mandlebrot(long double xoffset, long double yoffset, long double zoom)
{

    // long double Min_rangeX = -2.5+xoffset-(zoom/2.5);

    //long double Min_rangeX = -2.5+xoffset-(zoom*2.5);
    //long double Max_rangeX = 1.0+xoffset+zoom;
    //long double Min_rangeY = -1.0+yoffset-zoom;
    //long double Max_rangeY = 1.0+yoffset+zoom;

    long double Min_rangeX = xoffset-(zoom*2.5);
    long double Max_rangeX = xoffset+zoom;
    long double Min_rangeY = yoffset-zoom;
    long double Max_rangeY = yoffset+zoom;


////////--------------------------

    int iteration = 0;


    long double xtemp = 0;
    long double ytemp = 0;

    long double xVal = 0;
    long double yVal = 0;

    long double scaledColour = 0;

    sf::Color color(255, 255, 255);
    sf::Color color1(40,40,150);
    sf::Color color2(200,60,60);

    sf::Image picture({ScreenWidth, ScreenHeight}, sf::Color::Black);

    for(int x = 1; x<ScreenWidth-1; x++)
    {
        for(int y = 1; y<ScreenHeight-1; y++)
        {

            long double scaledX = scaleRange(0.0, ScreenWidth, Min_rangeX, Max_rangeX, x);
            long double scaledY = scaleRange(0.0, ScreenHeight, Min_rangeY, Max_rangeY, y);

            iteration = 0;
            xVal = 0;
            yVal = 0;

            while(xVal*xVal+yVal*yVal <= 4 && iteration< max_iteration)
            {

                xtemp = xVal*xVal-yVal*yVal + scaledX;
                yVal = 2*xVal*yVal + scaledY;
                xVal = xtemp;

                iteration++;

//cout<<scaledColour<<" "<<iteration<<endl;

//cout<<xVal*xVal+yVal*yVal<<" "<<x<<" "<<y<<" "<<(int)color.r<<" "<<(int)color.g<<" "<<(int)color.b<<endl;

            }

            if(iteration >= max_iteration)
            {
// this removes colour from the center
// if you remove this top portion the center will be filled with colour
                color.r = 0;
                color.g = 0;
                color.b = 0;
                picture.setPixel({x, y}, color);

            }
            else
            {

                scaledColour = scaleRange(0.0, max_iteration, 0.0, 255, iteration);

                if(iteration<max_iteration/2)
                {
                    color.r = (int)scaleRange(0.0, 255*255, 0.0, 255, scaledColour*scaledColour);
                    color.g = (int)scaleRange(0.0, 255*255, 0.0, 255, scaledColour*scaledColour);
                    color.b = (int)scaleRange(0.0, sqrt(255), 0.0, 255, sqrt(scaledColour));
                }
                else
                {
                    color.r = (int)scaleRange(0.0, sqrt(255), 0.0, 255, sqrt(scaledColour));
                    color.g = (int)scaleRange(0.0, 255*255, 0.0, 255, scaledColour*scaledColour);
                    color.b = (int)scaleRange(0.0, 255*255, 0.0, 255, scaledColour*scaledColour);
                }


                color = linearInterpolation(color1,color2,iteration);

                picture.setPixel({x, y}, color);
                //black and white
                //color.r = (int)scaledColour;
                //color.g = (int)scaledColour;
                //color.b = (int)scaledColour;


            }

        }
    }

    //  picture.saveToFile("result.png");

    return picture;
}



void renderingThread(sf::RenderWindow* window)
{
    window->setActive(true);
    // window->setFramerateLimit(60);
    // the rendering loop
    while (window->isOpen())
    {
        // draw...
        window->clear();
        window->setView(view);


        window->draw(Background);


        sf::View view = window->getView();
        window->display();
    }
}




int main()
{

//--
//ToDO

// make zoom and move progressively smaller to avoid flipping

//use exponential function 2^(-0.1x)

//--

    sf::RenderWindow window(sf::VideoMode({ScreenWidth, ScreenHeight}), "mandlebrot");
    //window.setFramerateLimit(60);

    pictures = mandlebrot(0,0,1);
    MapTexture.update(pictures);
    // picture.saveToFile("result.png");

    bool next = false;
    long double movex = 0.0;
    long double movey = 0.0;
    long double zoom = 1.0;

    //int multiplierIter = 0;
    long double multiplierIter = 1.0;

    //float multiplier = pow(2.0,(-0.1*multiplierIter));
    //float multiplier = 1.0/multiplierIter;
    long double multiplier = 1.0/pow(2,multiplierIter-1);


    window.setActive(false);
//    sf::Thread thread(&renderingThread, &window);
    //  thread.launch();


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
                view = getLetterboxView( view, resized->size.x, resized->size.y );
            }
        }


        // multiplier = pow(2.0,(-1*(multiplierIter+5)));

        //  multiplier = 1.0/multiplierIter;
        multiplier = 1.0/pow(2,multiplierIter-1);

        //   multiplier = multiplierIter;


        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            next = true;
            movex-=multiplier;
            //movex-=0.1*multiplier;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            next = true;
            movex+=multiplier;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            next = true;
            movey-=multiplier;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            next = true;
            movey+=multiplier;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add))
        {
            next = true;
            multiplierIter++;

            //zoom-=0.1*multiplier;
            zoom=multiplier;

        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract))
        {
            next = true;
            multiplierIter--;
            if(multiplierIter <1.0)
            {
                multiplierIter = 1.0;
                zoom = 1.0;
            }

            zoom=multiplier;
            //zoom+=0.1*multiplier;

        }




        if(next)
        {
            sf::Image pictures = mandlebrot(movex,movey,zoom);

            MapTexture.update(pictures);

            next = false;
            cout<<multiplier<<" "<<multiplierIter<<" "<<zoom<<endl;
        }



        window.clear();
        window.setView(view);


        window.draw(Background);


        sf::View view = window.getView();
        window.display();


    }

    return 0;
}
