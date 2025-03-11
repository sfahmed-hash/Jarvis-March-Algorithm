1. Find the leftmost point (smallest x-coordinate). This will always be part of the convex hull.
2. Select the next point in a counterclockwise direction by checking the orientation of all other points.
3. Repeat step 2 until returning to the starting point.


LEFT CLICK ON THE SCREEN TO CREATE POINTS

press C → Compute Convex Hull.
press S → Stepwise execution.
press Right Arrow → Next step.
press A → Animate convex hull.
press R → Reset.


COMPILE USING THE COMMAND-

g++ test.cpp -o test -lsfml-graphics -lsfml-window -lsfml-system


EXECUTE USING THE COMMAND-

./test
