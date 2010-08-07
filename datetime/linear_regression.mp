beginfig(1);
draw (0,0)--(100,0);
draw (0,0)--(0,100);
for m=0 upto 11:
dotlabel.bot("", (m*20,0));
endfor
draw (-5,-5)--(80, 122);
pickup pencircle scaled 2pt;
drawdot(00,04) withcolor 0.6white;

drawdot(20,30);
drawdot(20,34) withcolor 0.6white;

drawdot(40,57);
drawdot(40,61) withcolor 0.6white;

drawdot(60,90);
drawdot(60,94) withcolor 0.6white;
pickup pencircle scaled 0.6pt;
draw fullcircle scaled 1.7pt shifted (00, 08);
draw fullcircle scaled 1.7pt shifted (20, 38);
draw fullcircle scaled 1.7pt shifted (40, 65);
draw fullcircle scaled 1.7pt shifted (60, 98);
endfig;

beginfig(2);
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

