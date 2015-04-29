// Binary to decimal on HEX displays found online at:
//		http://www.eng.utah.edu/~nmcdonal/Tutorials/BCDTutorial/BCDConversion.html

module bin2dec(
	input [31:0] iData,
	output reg [3:0] HunThousand,
	output reg [3:0] TenThousand,
	output reg [3:0] Thousands,
	output reg [3:0] Hundreds,
	output reg [3:0] Tens,
	output reg [3:0] Ones
	);
	
	integer i;
	always@(iData)
	begin
		HunThousand	= 4'd0;
		TenThousand	= 4'd0;
		Thousands	= 4'd0;
		Hundreds 	= 4'd0;
		Tens 			= 4'd0;
		Ones			= 4'd0;
		
		for (i = 31; i >= 0; i = i - 1)
		begin
			// Add 3 to columns
			if (HunThousand >= 5)
				HunThousand = HunThousand + 3;
			if (TenThousand >= 5)
				TenThousand = TenThousand + 3;
			if (Thousands >= 5)
				Thousands = Thousands + 3;
			if (Hundreds >= 5)
				Hundreds = Hundreds + 3;
			if (Tens >= 5)
				Tens = Tens + 3;
			if (Ones >= 5)
				Ones = Ones + 3;
				
			// Shift left one
			HunThousand = HunThousand << 1;
			HunThousand[0] = TenThousand[3];
		
			TenThousand = TenThousand << 1;
			TenThousand[0] = Thousands[3];
			
			Thousands = Thousands << 1;
			Thousands[0] = Hundreds[3];
			
			Hundreds = Hundreds << 1;
			Hundreds[0] = Tens[3];
			
			Tens = Tens << 1;
			Tens[0] = Ones[3];
			
			Ones = Ones << 1;
			Ones[0] = iData[i];
		end
	end
endmodule