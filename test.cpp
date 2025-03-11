#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <sstream>

struct Point {
    float x, y;
};

std::vector<Point> points;
std::vector<Point> hull;
std::vector<std::pair<Point, Point>> stepLines;
int stepIndex = 0;
bool stepping = false, animating = false, computing = false;
int p = -1, q = -1;
int leftmost = -1;
bool highlightCurrent = false;

sf::Font font;

// Function to determine orientation
int orientation(Point p, Point q, Point r) {
    //CROSS PRODUCT: y2.x1 - y1.x2
    float val = (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
    if (val == 0) return 0;  // Collinear
    return (val > 0) ? -1 : 1;  // ACW or CW
}

// Reset function
void reset() {
    points.clear();
    hull.clear();
    stepLines.clear();
    stepIndex = 0;
    stepping = false;
    animating = false;
    computing = false;
    p = q = -1;
    leftmost = -1;
    highlightCurrent = false;
}

// Compute the convex hull in step mode initialization
void initStepMode() {
    hull.clear();
    stepLines.clear();
    if (points.size() < 3) return;
    leftmost = 0;
    for (int i = 1; i < points.size(); i++) {
        if (points[i].x < points[leftmost].x)
            leftmost = i;
    }
    p = leftmost;
    q = -1;
    hull.push_back(points[p]);
    stepIndex = 0;
    highlightCurrent = true;
}

// Step through one iteration of the convex hull computation
void stepConvexHull() {
    if (p == -1 || points.size() < 3) return;
    if (q == -1) q = (p + 1) % points.size();
    
    if (stepIndex < points.size()) {
        stepLines.push_back({points[p], points[stepIndex]});
        // Compare orientation between current candidate and step candidate
        if (orientation(points[p], points[stepIndex], points[q]) == -1) {
            q = stepIndex;
        }
        stepIndex++;
    } else {
        hull.push_back(points[q]);
        p = q;
        if (p == leftmost) {
            stepping = false;
            highlightCurrent = false;
            return;
        }
        q = -1;
        stepIndex = 0;
    }
}

// Jarvis March Algorithm (all at once)
void computeConvexHull() {
    hull.clear();
    if (points.size() < 3) return;

    int leftmost = 0;
    for (int i = 1; i < points.size(); i++) {
        if (points[i].x < points[leftmost].x)
            leftmost = i;
    }

    int p = leftmost, q;
    do {
        hull.push_back(points[p]);
        q = (p + 1) % points.size();
        for (int i = 0; i < points.size(); i++) {
            if (orientation(points[p], points[i], points[q]) == -1)
                q = i;
        }
        p = q;
    } while (p != leftmost);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Convex Hull Visualization");
    sf::Clock animationClock;

    if (!font.loadFromFile("ARIAL.TTF")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    points.push_back({(float)event.mouseButton.x, (float)event.mouseButton.y});
                    if (computing)
                        computeConvexHull();
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::C) {
                    computeConvexHull();
                    computing = true;
                    stepping = animating = false;
                }
                if (event.key.code == sf::Keyboard::S) {
                    stepping = true;
                    animating = false;
                    initStepMode();
                }
                if (event.key.code == sf::Keyboard::Right && stepping) {
                    stepConvexHull();
                }
                if (event.key.code == sf::Keyboard::A) {
                    stepping = false;
                    animating = true;
                    initStepMode();
                }
                if (event.key.code == sf::Keyboard::R) {
                    reset();
                }
            }
        }

        if (animating && animationClock.getElapsedTime().asSeconds() > 0.5f) {
            stepConvexHull();
            animationClock.restart();
        }

        window.clear(sf::Color::Black);

        // Draw points and their coordinates
        for (const auto& p : points) {
            sf::CircleShape point(5);
            point.setPosition(p.x - 5, p.y - 5);
            point.setFillColor(sf::Color::White);
            window.draw(point);
            
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(12);
            text.setFillColor(sf::Color::Cyan);
            std::stringstream ss;
            ss << "(" << (int)p.x << ", " << (int)p.y << ")";
            text.setString(ss.str());
            text.setPosition(p.x + 5, p.y - 10);
            window.draw(text);
        }

        // Draw step lines (the candidate lines during step mode)
        sf::VertexArray stepLinesArray(sf::Lines);
        for (const auto& line : stepLines) {
            stepLinesArray.append(sf::Vertex(sf::Vector2f(line.first.x, line.first.y), sf::Color::Green));
            stepLinesArray.append(sf::Vertex(sf::Vector2f(line.second.x, line.second.y), sf::Color::Green));
        }
        window.draw(stepLinesArray);

        // Draw the convex hull (the finalized part)
        sf::VertexArray lines(sf::LinesStrip);
        for (const auto& p : hull) {
            lines.append(sf::Vertex(sf::Vector2f(p.x, p.y), sf::Color::Red));
        }
        if (!hull.empty()) 
            lines.append(sf::Vertex(sf::Vector2f(hull[0].x, hull[0].y), sf::Color::Red));
        window.draw(lines);

        // ------- Draw the side panel with orientation info -------
        // Panel background on the right side (200 pixels wide)
        sf::RectangleShape sidePanel(sf::Vector2f(200, 600));
        sidePanel.setFillColor(sf::Color(50, 50, 50, 200));
        sidePanel.setPosition(600, 0);
        window.draw(sidePanel);

        // Prepare orientation info text
        std::stringstream info;
        info << "Orientation Info:\n\n";

        if (stepping && points.size() >= 3) {
            info << "Current p: (" << (int)points[p].x << ", " << (int)points[p].y << ")\n";
            if (stepIndex < points.size()) {
                info << "Candidate: (" << (int)points[stepIndex].x << ", " << (int)points[stepIndex].y << ")\n";
                if (q != -1) {
                    int orientVal = orientation(points[p], points[stepIndex], points[q]);
                    std::string orientStr;
                    if (orientVal == 0)
                        orientStr = "Collinear";
                    else if (orientVal > 0)
                        orientStr = "Clockwise";
                    else
                        orientStr = "Counterclockwise";
                    info << "Orientation: " << orientStr << "\n";
                } else {
                    info << "Orientation: N/A\n";
                }
            } else {
                info << "Candidate: -\n";
                info << "Orientation: -\n";
            }
            info << "Current q: ";
            if (q != -1)
                info << "(" << (int)points[q].x << ", " << (int)points[q].y << ")\n";
            else
                info << "None\n";
        } else if (computing) {
            info << "Computed Convex Hull\n";
        } else {
            info << "Press:\n"
                 << "  C - Compute Hull\n"
                 << "  S - Step Mode\n"
                 << "  A - Animate Steps\n"
                 << "  R - Reset\n";
        }

        sf::Text panelText;
        panelText.setFont(font);
        panelText.setCharacterSize(14);
        panelText.setFillColor(sf::Color::White);
        panelText.setString(info.str());
        // Place text with some margin inside the panel
        panelText.setPosition(610, 10);
        window.draw(panelText);
        // ------- End side panel -------

        window.display();
    }

    return 0;
}
