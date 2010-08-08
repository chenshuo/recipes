filenametemplate "%j-%c.mps";
prologues := 3;

beginfig(1);
draw (0,0)--(95,0);
%draw (0,0)--(0,120);
draw (0,0)--(0,16);
draw (0,19)--(0,45);
draw (0,48)--(0,75);
draw (0,78)--(0,110);
label(btex $\approx$ etex, (0, 17));
label(btex $\approx$ etex, (0, 46));
label(btex $\approx$ etex, (0, 76));
for m=0 upto 4:
  dotlabel.bot(decimal(m), (m*20,0));
endfor
draw (-5,-5)--(75, 115) withcolor blue;
pickup pencircle scaled 2pt;
drawdot(20,30);
drawdot(40,57);
drawdot(60,90);

label.lft("0", (0, 0));
label.lft("1", (0, 8));
label.lft("31", (0, 30));
label.lft("32", (0, 38));
label.lft("61", (0, 57));
label.lft("62", (0, 65));
label.lft("92", (0, 90));
label.lft("93", (0, 98));
drawdot(0,30);
drawdot(0,38);
drawdot(0,57);
drawdot(0,65);
drawdot(0,90);
drawdot(0,98);
pickup pencircle scaled 0.6pt;
draw fullcircle scaled 1.7pt shifted (00, 08);
draw fullcircle scaled 1.7pt shifted (20, 38);
draw fullcircle scaled 1.7pt shifted (40, 65);
draw fullcircle scaled 1.7pt shifted (60, 98);
pickup pencircle scaled 0.5pt;
draw (0,30)--(19,30) dashed evenly withcolor 0.6white;
draw (0,38)--(19,38) dashed evenly withcolor 0.6white;
draw (0,57)--(39,57) dashed evenly withcolor 0.6white;
draw (0,65)--(39,65) dashed evenly withcolor 0.6white;
draw (0,90)--(59,90) dashed evenly withcolor 0.6white;
draw (0,98)--(59,98) dashed evenly withcolor 0.6white;
endfig;

beginfig(2);
draw (0,0)--(95,0);
%draw (0,0)--(0,120);
draw (0,0)--(0,16);
draw (0,19)--(0,45);
draw (0,48)--(0,75);
draw (0,78)--(0,110);
label(btex $\approx$ etex, (0, 17));
label(btex $\approx$ etex, (0, 46));
label(btex $\approx$ etex, (0, 76));
for m=0 upto 4:
  dotlabel.bot(decimal(m), (m*20,0));
endfor
draw (-5,-5)--(75, 115) withcolor blue;
pickup pencircle scaled 2pt;
drawdot(00,04) withcolor red;

drawdot(20,30);
drawdot(20,34) withcolor red;

drawdot(40,57);
drawdot(40,61) withcolor red;

drawdot(60,90);
drawdot(60,94) withcolor red;

label.lft("0.5", (0, 4));
label.lft("31.5", (0, 34));
label.lft("61.5", (0, 61));
label.lft("92.5", (0, 94));
drawdot(0,34);
drawdot(0,61);
drawdot(0,94);

pickup pencircle scaled 0.6pt;
draw fullcircle scaled 1.7pt shifted (00, 08);
draw fullcircle scaled 1.7pt shifted (20, 38);
draw fullcircle scaled 1.7pt shifted (40, 65);
draw fullcircle scaled 1.7pt shifted (60, 98);

pickup pencircle scaled 0.5pt;
draw (0,34)--(19,34) dashed evenly withcolor 0.6white;
draw (0,61)--(39,61) dashed evenly withcolor 0.6white;
draw (0,94)--(59,94) dashed evenly withcolor 0.6white;

endfig;

beginfig(3);
draw (0,0)--(400,0) withcolor 0.7white;
draw (0,0)--(0,350) withcolor 0.7white;

pickup pencircle scaled 2pt;
for m=0 upto 11:
  drawdot(30*m, 0);
endfor
pickup pencircle scaled 1pt;
drawdot(30*1, 31);
drawdot(30*2, 61);
drawdot(30*3, 92);
drawdot(30*4, 122);
drawdot(30*5, 153);
drawdot(30*6, 184);
drawdot(30*7, 214);
drawdot(30*8, 245);
drawdot(30*9, 275);
drawdot(30*10, 306);

drawdot(30*1, 32);
drawdot(30*2, 62);
drawdot(30*3, 93);
drawdot(30*4, 123);
drawdot(30*5, 154);
drawdot(30*6, 185);
drawdot(30*7, 215);
drawdot(30*8, 246);
drawdot(30*9, 276);
drawdot(30*10, 307);

pickup pencircle scaled 0.5pt;
draw(0,0.4)--(11*30,337) withcolor 0.6white;
endfig;

end

