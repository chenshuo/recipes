x=0:11
d=[31 30 31 30 31 31 30 31 30 31 31]
s=[0 cumsum(d)]
y=s+0.5
n=12
a=(n*sum(x.*y) - sum(x)*sum(y)) / (n*sum(x.*x) - sum(x)^2)
b=mean(y)-a*mean(x)

