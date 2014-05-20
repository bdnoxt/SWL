%
% using Craig's notation
%	Craig's book pp. 79
%

%addpath('../../src/robot_kinematics');

dof = 6;

%------------------------------------------------------------------------------
a2 = 431.8;
a3 = 20.32;
d3 = 149.09;
d4 = 433.07;

%------------------------------------------------------------------------------
T0 = eye(4);
T1 = eye(4);
T2 = [
	1	0	0	0
	0	0	1	0
	0	-1	0	0
	0	0	0	1
];
T3 = [
	1	0	0	a2
	0	0	1	d3
	0	-1	0	0
	0	0	0	1
];
T4 = [
	1	0	0	a2 + a3
	0	-1	0	d3
	0	0	-1	-d4
	0	0	0	1
];
T5 = [
	1	0	0	a2 + a3
	0	0	1	d3
	0	-1	0	-d4
	0	0	0	1
];
T6 = [
	1	0	0	a2 + a3
	0	-1	0	d3
	0	0	-1	-d4
	0	0	0	1
];

T = cell(1, dof + 1);
T{1} = T0;
T{2} = T1;
T{3} = T2;
T{4} = T3;
T{5} = T4;
T{6} = T5;
T{7} = T6;

%------------------------------------------------------------------------------
a = zeros(dof, 1);
alpha = zeros(dof, 1);
d = zeros(dof, 1);
theta = zeros(dof, 1);

for idx = 1:dof
	[ a(idx), alpha(idx), d(idx), theta(idx) ] = calc_dh(T{idx}, T{idx+1}, 'craig');
end;
dh_param1 = [ a, alpha, d, theta ];

%------------------------------------------------------------------------------

dh_param2 = calc_dh_param(T, 'craig');
