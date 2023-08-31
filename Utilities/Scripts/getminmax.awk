BEGIN { min = max = "NaN" }
{
    min = (NR==1 || $2<min ? $2 : min)
    max = (NR==1 || $3>max ? $3 : max)
}
END { print $1, min, max }

