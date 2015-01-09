`ifndef MM_DEFINES_INCLUDED
`define MM_DEFINES_INCLUDED

`define endian_swap(a) {a[7:0], a[15:8], a[23:16], a[31:24]}

function integer log2;
input [31:0] value;
reg [31:0] i;
reg [31:0] t;
begin
   t = 1;
   for (i = 0; t < value; i = i + 1)  
	t = t << 1;
   log2 = i;
end
endfunction
`endif
