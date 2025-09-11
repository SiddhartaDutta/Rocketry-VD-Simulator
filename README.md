# Rocketry-VD-Simulator
A simulator to understand trajectory estimation, control post-launch navigation, and enhance post-launch control with machine learning techniques. 

This project combines a handmade physics engine with reinforcement learning. I custom built my own physics engine to model a stage 1 booster (for now SpaceX's). With data publicly available, I have compiled data to create a reasonable and generally accurate depiction of a Falcon 9's stage 1 booster. Using my physics library, I have then found the ability to simulate how the booster would behave in it's flight down to the surface. To aid it, I am first exploring creating my own reinforcement learning model to guide the rocket to a soft landing, before exploring other solutions such as PID control loops with integrated Kalman filters.

More to come as this project continues!
