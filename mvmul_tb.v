module mvmul_tb();


	// Start debug wires and ports
	reg [0:0] rst;
	reg [0:0] clk;
	reg [0:0] in_set_mem_phase;
	reg [0:0] in_run_phase;
	reg [0:0] in_check_mem_phase;
	reg [31:0] clocks_in_set_mem_phase;
	reg [31:0] clocks_in_run_phase;
	reg [31:0] clocks_in_check_mem_phase;
	reg [31:0] num_clocks_after_reset;
	reg [31:0] total_cycles;
	reg [31:0] max_cycles;
	reg [4:0] raddr_0;
	wire [31:0] rdata_0;
	reg [4:0] raddr_1;
	wire [31:0] rdata_1;
	reg [4:0] dbg_wr_addr;
	reg [31:0] dbg_wr_data;
	reg [0:0] dbg_wr_en;
	reg [4:0] dbg_addr;
	wire [31:0] dbg_data;
	reg [4:0] waddr_0;
	reg [31:0] wdata_0;
	reg [0:0] wen_0;
	wire [0:0] valid;

	initial begin
		#1 clk = 0;
		#1 rst = 1;
		#1 in_set_mem_phase = 1;
		#1 in_check_mem_phase = 0;
		#1 in_run_phase = 0;
		#1 total_cycles = 0;
		#1 max_cycles = 1000;
		#1 num_clocks_after_reset = 0;
		#1 clocks_in_set_mem_phase = 0;
		#1 clocks_in_run_phase = 0;
		#1 clocks_in_check_mem_phase = 0;
	end


	always @(posedge clk) begin
		total_cycles <= total_cycles + 1;
	end

	always @(posedge clk) begin
		if (total_cycles >= max_cycles) begin if (valid == 1 && in_check_mem_phase) begin $display("Passed"); $finish(); end else begin $display("valid == %d. Ran out of cycles, finishing.", valid); $finish(); end end
	end

	always @(posedge clk) begin
		if (!in_set_mem_phase) begin num_clocks_after_reset <= num_clocks_after_reset + 1; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase) begin clocks_in_set_mem_phase <= clocks_in_set_mem_phase + 1; end 
	end

	always @(posedge clk) begin
		if (in_check_mem_phase) begin if (!valid) begin $display("Failed: Checking memory, but the module is not done running"); $finish(); end end
	end

	always @(posedge clk) begin
		if (clocks_in_run_phase == (99)) begin in_check_mem_phase <= 1; in_run_phase <= 0; end
	end

	always @(posedge clk) begin
		if (in_run_phase) begin clocks_in_run_phase <= clocks_in_run_phase + 1; end
	end

	always @(posedge clk) begin
		if (in_check_mem_phase) begin clocks_in_check_mem_phase <= clocks_in_check_mem_phase + 1; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 0) begin dbg_wr_en <= 1; dbg_wr_addr <= 0; dbg_wr_data <= 6; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 1) begin dbg_wr_en <= 1; dbg_wr_addr <= 1; dbg_wr_data <= 1; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 2) begin dbg_wr_en <= 1; dbg_wr_addr <= 2; dbg_wr_data <= 2; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 3) begin dbg_wr_en <= 1; dbg_wr_addr <= 3; dbg_wr_data <= 3; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 4) begin dbg_wr_en <= 1; dbg_wr_addr <= 4; dbg_wr_data <= 7; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 5) begin dbg_wr_en <= 1; dbg_wr_addr <= 5; dbg_wr_data <= 5; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 6) begin dbg_wr_en <= 1; dbg_wr_addr <= 6; dbg_wr_data <= 5; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 7) begin dbg_wr_en <= 1; dbg_wr_addr <= 7; dbg_wr_data <= 2; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 8) begin dbg_wr_en <= 1; dbg_wr_addr <= 8; dbg_wr_data <= 9; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 9) begin dbg_wr_en <= 1; dbg_wr_addr <= 9; dbg_wr_data <= 9; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 10) begin dbg_wr_en <= 1; dbg_wr_addr <= 10; dbg_wr_data <= 3; end
	end

	always @(posedge clk) begin
		if (in_set_mem_phase && clocks_in_set_mem_phase == 11) begin dbg_wr_en <= 1; dbg_wr_addr <= 11; dbg_wr_data <= 7; end
	end

	always @(posedge clk) begin
		if (clocks_in_set_mem_phase == (11)) begin in_run_phase <= 1; rst <= 0; dbg_wr_en <= 0; in_set_mem_phase <= 0; end
	end

	always @(posedge clk) begin
		if (in_check_mem_phase && clocks_in_check_mem_phase == 0) begin dbg_addr <= 12; end
	end

	always @(posedge clk) begin
		if (in_check_mem_phase && clocks_in_check_mem_phase == 0) begin end
	end

	always @(posedge clk) begin
		if (in_check_mem_phase && clocks_in_check_mem_phase == 1) begin dbg_addr <= 13; end
	end

	always @(posedge clk) begin
		if (in_check_mem_phase && clocks_in_check_mem_phase == 1) begin $display("mem[%d] == %d", dbg_addr, dbg_data);  if (dbg_data == 71) begin $display("Correct."); end else begin $display("Assert failed"); $finish(); end end
	end

	always @(posedge clk) begin
		if (in_check_mem_phase && clocks_in_check_mem_phase == 2) begin dbg_addr <= 14; end
	end

	always @(posedge clk) begin
		if (in_check_mem_phase && clocks_in_check_mem_phase == 2) begin $display("mem[%d] == %d", dbg_addr, dbg_data);  if (dbg_data == 83) begin $display("Correct."); end else begin $display("Assert failed"); $finish(); end end
	end

	always @(posedge clk) begin
		if (in_check_mem_phase && clocks_in_check_mem_phase == 3) begin $display("mem[%d] == %d", dbg_addr, dbg_data);  if (dbg_data == 114) begin $display("Correct."); end else begin $display("Assert failed"); $finish(); end end
	end


	always #3 begin
		clk = !clk;
	end


	RAM3 ram(.clk(clk), .debug_addr(dbg_addr), .debug_data(dbg_data), .debug_write_addr(dbg_wr_addr), .debug_write_data(dbg_wr_data), .debug_write_en(dbg_wr_en), .raddr0(raddr_0), .raddr1(raddr_1), .raddr2(raddr_2), .rdata0(rdata_0), .rdata1(rdata_1), .rdata2(rdata_2), .rst(rst), .waddr(waddr_0), .wdata(wdata_0), .wen(wen_0));
	mvmul dut(.clk(clk), .raddr_0(raddr_0), .raddr_1(raddr_1), .rdata_0(rdata_0), .rdata_1(rdata_1), .rst(rst), .valid(valid), .waddr_0(waddr_0), .wdata_0(wdata_0), .wen_0(wen_0));
	// End debug wires and ports
endmodule