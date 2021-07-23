#include <iostream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>

using namespace std;

sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight) {

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

    if (horizontalSpacing) {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    }

    else {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport( sf::FloatRect(posX, posY, sizeX, sizeY) );

    return view;
}


float scaleRange(float Min_Range, float Max_Range, float Min_Num, float Max_num, float Num){

//this turns a number from (0 -> 1920) to (-2.5 -> 1)

return ((((Max_num-Min_Num)*(Num-Min_Range)) / (Max_Range-Min_Range))+Min_Num);

}


int main() {


//Range
float Min_rangeX = -2.5;
float Max_rangeX = 1.0;

float Min_rangeY = -1.0;
float Max_rangeY = 1.0;

int ScreenWidth = 1920;
int ScreenHeight = 1080;

int iteration = 0;
int max_iteration = 1000;

float xtemp = 0;
float ytemp = 0;

float xVal = 0;
float yVal = 0;

float scaledColour = 0;

sf::Color color(255, 255, 255);

 sf::Image picture;
picture.create(ScreenWidth, ScreenHeight, sf::Color::Black);

picture.setPixel(1, 1, sf::Color::Red);



for(int x = 1; x<ScreenWidth-1; x++){
for(int y = 1; y<ScreenHeight-1; y++){

float scaledX = scaleRange(0.0, ScreenWidth, Min_rangeX, Max_rangeX, x);
float scaledY = scaleRange(0.0, ScreenHeight, Min_rangeY, Max_rangeY, y);

iteration = 0;
xVal = 0;
yVal = 0;

while(xVal*xVal+yVal*yVal <= 4 && iteration< max_iteration){

xtemp = xVal*xVal-yVal*yVal + scaledX;
yVal = 2*xVal*yVal + scaledY;
xVal = xtemp;

iteration++;

//cout<<scaledColour<<" "<<iteration<<endl;

//cout<<xVal*xVal+yVal*yVal<<" "<<x<<" "<<y<<" "<<(int)color.r<<" "<<(int)color.g<<" "<<(int)color.b<<endl;

}
scaledColour = scaleRange(0.0, max_iteration, 0.0, 255, iteration);
color.r = (int)scaledColour;
color.g = (int)scaledColour;
color.b = (int)scaledColour;
picture.setPixel(x, y, color);
}
}

picture.saveToFile("result.png");

    sf::Texture MapTexture;
    MapTexture.loadFromImage(picture);

    sf::Sprite Background;
    Background.setTexture(MapTexture);
    Background.setPosition(sf::Vector2f(0, 0));



    sf::RenderWindow window(sf::VideoMode(1920, 1080), "SFML works!");
    window.setFramerateLimit(60);
    sf::View view(sf::FloatRect(0.f, 0.f, window.getSize().x, window.getSize().y));


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {

                window.close();
                return 69;
            }

            if (event.type == sf::Event::Resized) {

                view = getLetterboxView( view, event.size.width, event.size.height );

            }
        }

        window.clear();
        window.setView(view);


        window.draw(Background);


        sf::View view = window.getView();
        window.display();
    }

    return 0;
}
