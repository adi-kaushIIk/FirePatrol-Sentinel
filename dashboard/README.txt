FIREPATROL SENTINEL HTML DASHBOARD
=================================

Files:
- index.html   Command Center
- sensors.html Sensor Lab
- logs.html    Patrol Logs
- style.css    Shared styling
- app.js       Shared simulated state and interactions

QUICK START (recommended):
1. Extract this folder into your FirePatrolSentinel project, e.g. FirePatrolSentinel/dashboard/
2. Open the folder in VS Code.
3. Install/use the "Live Server" extension if needed.
4. Right-click index.html -> Open with Live Server.
5. Use the navigation links to move between all three pages.

Important:
- This is a front-end prototype. It does NOT connect to the ESP32 yet.
- Values are simulated, stored in this browser using localStorage, and shared between pages when served from the same origin.
- The firmware's current settings are reflected in the UI: obstacle threshold 30 cm, MQ-2 threshold 1800 (provisional), flame active LOW, target motor PWM 5%.
- Do not treat simulated readings as real fire safety information.
