FUN gcd(x,y)	IF x=y THEN x
		ELSE IF x>y THEN gcd(x-y, y)
		ELSE gcd(x, y-x) FI FI;
?gcd(10,25)