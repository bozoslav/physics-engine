#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <random>
#include <chrono>

using namespace sf;
using namespace std;

bool intersects(const RectangleShape &a, const RectangleShape &b)
{
    float aLeft = a.getPosition().x;
    float aTop = a.getPosition().y;
    float aRight = aLeft + a.getSize().x;
    float aBottom = aTop + a.getSize().y;

    float bLeft = b.getPosition().x;
    float bTop = b.getPosition().y;
    float bRight = bLeft + b.getSize().x;
    float bBottom = bTop + b.getSize().y;

    return (aLeft < bRight &&
            aRight > bLeft &&
            aTop < bBottom &&
            aBottom > bTop);
}

int main()
{
    RenderWindow window(sf::VideoMode({800, 600}), "test");
    window.setFramerateLimit(60);

    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> xDist(0, 800 - 30);
    uniform_int_distribution<int> yDist(0, 600 - 30);
    uniform_int_distribution<int> velocityXDist(-10, 10);
    uniform_int_distribution<int> velocityYDist(-2, 2);
    uniform_int_distribution<int> colorDist(0, 255);

    vector<pair<RectangleShape, Vector2f>> rects;
    for (int i = 0; i < 8; ++i)
    {
        RectangleShape rect(Vector2f(30.f, 30.f));
        rect.setFillColor(Color(colorDist(rng), colorDist(rng), colorDist(rng)));
        rect.setOutlineColor(Color::Black);
        rect.setOutlineThickness(1.f);
        rect.setPosition(Vector2f(xDist(rng), yDist(rng)));
        rect.setOrigin(rect.getSize() / 2.f);
        rects.emplace_back(rect, Vector2f(velocityXDist(rng), velocityYDist(rng)));
    }

    float g = 9.81f;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
                window.close();
        }

        for (auto &[rect, velocity] : rects)
        {
            float halfWidth = rect.getSize().x / 2.f;
            float halfHeight = rect.getSize().y / 2.f;

            if (rect.getPosition().y + halfHeight > window.getSize().y)
            {
                rect.setPosition(Vector2f(rect.getPosition().x, window.getSize().y - halfHeight));
                velocity.y = -velocity.y;
            }
            else if (rect.getPosition().y - halfHeight < 0)
            {
                rect.setPosition(Vector2f(rect.getPosition().x, halfHeight));
                velocity.y = -velocity.y;
            }

            if (rect.getPosition().x + halfWidth > window.getSize().x)
            {
                rect.setPosition(Vector2f(window.getSize().x - halfWidth, rect.getPosition().y));
                velocity.x = -velocity.x;
            }
            else if (rect.getPosition().x - halfWidth < 0)
            {
                rect.setPosition(Vector2f(halfWidth, rect.getPosition().y));
                velocity.x = -velocity.x;
            }

            velocity.y += g * 0.016f;
            rect.move(velocity);
        }

        for (size_t i = 0; i < rects.size(); ++i)
        {
            for (size_t j = i + 1; j < rects.size(); ++j)
            {
                if (intersects(rects[i].first, rects[j].first))
                {
                    float mA = 1.0f;
                    float mB = 1.0f;

                    Vector2f vA = rects[i].second;
                    Vector2f vB = rects[j].second;

                    Vector2f overlap = rects[i].first.getPosition() - rects[j].first.getPosition();

                    if (abs(overlap.x) > abs(overlap.y))
                    {
                        float penetration = (rects[i].first.getSize().x + rects[j].first.getSize().x) / 2 - abs(overlap.x);
                        if (overlap.x > 0)
                        {
                            rects[i].first.move(Vector2f(penetration / 2, 0));
                            rects[j].first.move(Vector2f(-penetration / 2, 0));
                        }
                        else
                        {
                            rects[i].first.move(Vector2f(-penetration / 2, 0));
                            rects[j].first.move(Vector2f(penetration / 2, 0));
                        }

                        float vA_x = vA.x * (mA - mB) / (mA + mB) + vB.x * (2 * mB) / (mA + mB);
                        float vB_x = vA.x * (2 * mA) / (mA + mB) + vB.x * (mB - mA) / (mA + mB);

                        rects[i].second.x = vA_x;
                        rects[j].second.x = vB_x;
                    }
                    else
                    {
                        float penetration = (rects[i].first.getSize().y + rects[j].first.getSize().y) / 2 - abs(overlap.y);
                        if (overlap.y > 0)
                        {
                            rects[i].first.move(Vector2f(0, penetration / 2));
                            rects[j].first.move(Vector2f(0, -penetration / 2));
                        }
                        else
                        {
                            rects[i].first.move(Vector2f(0, -penetration / 2));
                            rects[j].first.move(Vector2f(0, penetration / 2));
                        }

                        float vA_y = vA.y * (mA - mB) / (mA + mB) + vB.y * (2 * mB) / (mA + mB);
                        float vB_y = vA.y * (2 * mA) / (mA + mB) + vB.y * (mB - mA) / (mA + mB);

                        rects[i].second.y = vA_y;
                        rects[j].second.y = vB_y;
                    }
                }
            }
        }

        window.clear();

        for (const auto &[rect, _] : rects)
        {
            window.draw(rect);
        }

        window.display();
    }
}

// compile with: g++ main.cpp -I/opt/homebrew/Cellar/sfml/3.0.0_1/include/ -o app -L/opt/homebrew/Cellar/sfml/3.0.0_1/lib -lsfml-graphics -lsfml-window -lsfml-system -std=c++17
// :3