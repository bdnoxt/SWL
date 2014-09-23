function [A, S] = onmf_a(X, K, maxiter, tol, threshold)

[M, N] = size(X);

A = rand(M, K);
while min(A(:)) <= 0.0
	A = rand(M, K);
end;
A = A / norm(A,2);

S = rand(K, N);
while min(S(:)) <= 0.0
	S = rand(K, N);
end;
S = S / norm(S,2);

Ao = zeros(M, K);
So = zeros(K, N);

AS = zeros(M, N);
XSt = zeros(M, K);
ASXtA = zeros(M, K);
AtX = zeros(K, N);
AtAS = zeros(K, N);

for iter = 1:maxiter
	AS = A * S;
	AtX = A' * X;
	AtAS = A' * AS;
	XSt = X * S';
	ASXtA = AS * AtX';
	Ao = A;
	So = S;
	for ii = 1:M
		for jj = 1:K
			A(ii,jj) = A(ii,jj) * XSt(ii,jj) / ASXtA(ii,jj);
		end;
	end;
	for ii = 1:K
		for jj = 1:N
			S(ii,jj) = S(ii,jj) * AtX(ii,jj) / AtAS(ii,jj);
		end;
	end;

	% TODO [check] >> which one is correct?
	%A = A / norm(A,2);
	for jj = 1:K
		A(:,jj) = A(:,jj) / norm(A(:,jj));
	end;
	%S = S / norm(S,2);

	if norm(A - Ao, 2) < tol && norm(S - So, 2) < tol
		break;
	end;
end;