class ram {

  input_32 wdata;
  input_5 waddr;
  input_1 wen;

  void write(bit_5 addr, bit_32 data) {
  set_wen:
    set_port(wen, 1);
  set_wdata:
    set_port(wdata, data);
  set_waddr:
    set_port(waddr, addr);

    add_constraint(start(set_wen) == start(set_wdata));
    add_constraint(start(set_wen) == start(set_waddr));
    add_constraint(start(set_wdata) + 3 == end(set_waddr));
  }

  bit_32 read(bit_5 addr) {
  set_addr:
    set_port(raddr, addr);

    bit_32 res;

  read_data:
    res = read_port(rdata);

    add_constraint(start(read_data) - 1 == end(set_addr));
  }

};

void filter_ram(ram* mem) {
  sint_32 i;
  bit_32 a;
  bit_32 b;
  for (i = 0; i < 10; i = i + 1) {
    a = ram->read(i) + ram->read(i + 1);
    ram->write(i + 10);
  }
}
