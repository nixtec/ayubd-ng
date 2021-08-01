#!/usr/bin/awk -f

BEGIN {
  FS = "[\r\n]+";
  count = 0;
}

{
  count++;
}

END {
  print "Total " count " lines found";
}
