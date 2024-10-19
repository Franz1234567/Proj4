import matplotlib.pyplot as plt

# Data for the 2000 input case
speed_2000 = [33, 906, 1196, 1362, 1496, 1598, 1667, 1720, 1762, 1797, 1827, 1849, 1870, 1888, 1900, 1917, 1921, 1933, 1942, 1947, 1950, 1959, 1965, 1966, 1973, 1977, 1979, 1979, 1979, 1991, 1991, 1989, 1991, 1991, 1955, 1995, 1988, 1998, 1999]
u_2000 = [46, 47, 52, 57, 61, 64, 66, 69, 71, 72, 74, 75, 76, 77, 78, 79, 80, 80, 81, 81, 82, 82, 82, 83, 83, 83, 83, 83, 84, 84, 84, 84, 84, 84, 85, 84, 85, 85, 85]


# Data for the 1500 input case
speed_1500 = [0, 57, 1018, 1051, 1151, 1262, 1323, 1373, 1409, 1405, 1437, 1456, 1462, 1463, 1487, 1487, 1494, 1499, 1488, 1492, 1494, 1493, 1493, 1494, 1493, 1493, 1502, 1510]
u_1500 = [35, 37, 41, 45, 48, 49, 50, 50, 51, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53]


# Plot Speed for 2000 input case
plt.figure(figsize=(6, 4))
plt.plot(speed_2000, 'o-', label='Reference Speed 2000', color='blue')
plt.axhline(y=2000, color='red', linestyle='--', label='Target 2000')  # Add red line at reference speed 2000
plt.xlabel('Seconds')
plt.ylabel('Speed')
plt.title('Speed evolution for 2000 target')
plt.grid(True)
plt.legend()
plt.show()

# Plot PWM (u) for 2000 input case
plt.figure(figsize=(6, 4))
plt.plot(u_2000, 'o-', label='PWM u 2000', color='blue')
plt.xlabel('Seconds')
plt.ylabel('PWM u')
plt.title('PWM evolution for 2000 target')
plt.grid(True)
plt.legend()
plt.show()

# Plot Speed for 1500 input case
plt.figure(figsize=(6, 4))
plt.plot(speed_1500, 'o-', label='Reference Speed 1500', color='blue')
plt.axhline(y=1500, color='red', linestyle='--', label='Target 1500')  # Add red line at reference speed 1500
plt.xlabel('Seconds')
plt.ylabel('Speed')
plt.title('Speed evolution for 1500 target')
plt.grid(True)
plt.legend()
plt.show()

# Plot PWM (u) for 1500 input case
plt.figure(figsize=(6, 4))
plt.plot(u_1500, 'o-', label='PWM u 1500', color='blue')
plt.xlabel('Seconds')
plt.ylabel('PWM u')
plt.title('PWM evolution for 1500 target')
plt.grid(True)
plt.legend()
plt.show()
