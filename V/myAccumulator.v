// Accumulator module
// Adds up all ROW and COL values in verilog
//	So the HPS can request it rather than add up everything in software

module myAccumulator (
	input 	[9:0] 	iData,
	output 	[9:0] 	oData,
	input					iStart,
	input		[9:0]		level,
	input					iCLK,
	input					iRST,
	input 				iRW,
	input		[9:0]		iRow,
	input		[9:0]		iCol,
	input					iDVAL
	);
	
reg 	[9:0]	rowMem[479:0];  
reg 	[9:0]	colMem[639:0];

wire value;
assign value = (iData > level[8:1]) ? 1 : 0;

always@(posedge iCLK)
begin
	if (iStart == 1)
	begin
		if (iRow == 0) rowMem[iRow] <= value;
		else rowMem[iRow] <= rowMem[iRow] + value;
		
		if (iCol == 0) colMem[iCol] <= value;
		else colMem[iCol] <= colMem[iCol] + value;
	end
end


endmodule 