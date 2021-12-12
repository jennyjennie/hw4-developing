//&S-
//&T-

PrintBinUnConstantInvocation;

// print, binary, unary operator, function invocation that only use constant value as expr

func5(b, c: real);
func()
begin
    print -1;
    print 20 + 30;
end
end

begin
    var a: array 2 of array 3 of integer;
    var b: real;
    a[1] := 1234;
    print a[40][50][60];
    print a;
    print 1 * 5;
    print 12 / 3;
    print 17 mod 9;
    print -1 + 8;
    print 15 - 15;
    print 10 < 4;
    print 10 <= 4;
    print 10 = 4;
    print 10 >= 4;
    print 10 <> 4;
    print not true;
    print 10 and 0;
    print 1 or 0;
    print 1 + 8 * 4;
    print (1 + 8) * 4;
    print -(1 + 8.2) * 4;
    print func();
end
end
