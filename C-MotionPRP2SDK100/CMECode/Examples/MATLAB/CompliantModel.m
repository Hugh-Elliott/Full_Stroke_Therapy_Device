% Using our system

Tcoulomb = 10.95;
TNm = 0.144761812;

%Material constants
En=6000000;                 %Young's Modulus of the belt material in [Pa](Neoprene)

t = 2.8/1000;            % belt thickness in meters
w = 14/1000;             % belt width in meters
a = t*w;   % belt cross sectional area in m^2

l3 = 0.5588 + 12.6/1000;        % Approximate length of belt between pulleys in meters

l1 = l3/2;                      % Approximate length of belt when car is at the center
l2 = l1;

k1 = a*En/l1;                                 %stiffness between left pully and car (Neoprene)
k2 = a*En/l2;                                 %stiffness between car and right pully (Neoprene)
k3 = a*En/l3;                                 %stifness in lower part of the belt between the two pullies (Neoprene)

r = 0.011777;                        %Pulley radius in meters

%Friction constants b
% B = 0.0;               %friction constant for the pullies (linear friction
                        %assumed) [N*m*s]
% bf = 0.0;               %Friction constant for the car on the track [(N*s)/m]

%Inertia and Mass
J = 2.7685*10^(-6);              % pully inertia in [Kg*m^2]
J1 = J;
J2 = J;
% total inertia = 0.00001605
M = .17982;                %car mass in [Kg]

Kt = 0.089999478; %       %Torque constant in [N*m/A]
Ke = Kt;                      %Back-EMF constant in [V/(rad*s)]
R = 1.8;                     %Resistance in [Ohm]

% Nonlinear Friction
B1 = 1.5*0.0007014; % Nm s
B2 = B1; % Nm s
% B = 9.873644121; % (N s)/m
% B = 1.0*19.0424; % (N s)/m
B = 1/0.098736441;
Tauc_1 = 1*10.95*TNm/100; % Nm
Tauc_2 = Tauc_1; % Nm
Fc = 1.0*10.95; % N, not sure how accurate this is

% mtemp = spline(time, motCom, new_time);
% motComIn = timeseries(mtemp.', new_time);

load_system('CompliantModelSim.slx');
set_param('CompliantModelSim', 'StopTime', num2str(max(time)));
outCompliant = sim('CompliantModelSim.slx');
