class fifo {
  input_1 read_valid;
  output_1 read_ready;
  input_32 in_data;

  input_1 write_valid;
  output_1 write_ready;
  output_32 out_data;

  void defaults() {
    write_port(read_valid, 0);
    write_port(write_valid, 0);    
  }

  bit_32 read_fifo() {
  stall_ready: stall(read_port(read_ready));
  set_valid: write_port(read_valid, 1);

    bit_32 out_val;
  set_out_rf: out_val = read_port(out_data);
  ret: return out_val;

    add_constraint(end(stall_ready) < start(set_valid));
    add_constraint(end(set_valid) + 1 == start(ret));
    add_constraint(start(set_out_rf) == start(ret));    
  }

  void write_fifo(bit_32& data) {
  stall_ready: stall(read_port(write_ready));
  set_valid: write_port(write_valid, 1);
  set_data: write_port(in_data, data);
  ret: return;

    add_constraint(end(stall_ready) < start(set_valid));
    add_constraint(start(set_valid) == start(set_data));
    add_constraint(end(set_data) + 1 == start(ret));
  }

};

class eth_axis_tx {

public:
  input_1 s_eth_hdr_valid;
  output_1 s_eth_hdr_ready;
  input_48 s_eth_dest_mac;
  input_48 s_eth_src_mac;
  input_16 s_eth_type;

  input_8 s_eth_payload_axis_tdata;
  input_1 s_eth_payload_axis_tvalid;
  output_1 s_eth_payload_axis_tready;
  input_1 s_eth_payload_axis_tlast;
  input_1 s_eth_payload_axis_tuser;

  output_1 busy;

  input_1  m_axis_tready;

  void defaults() {
    write_port(s_eth_hdr_valid, 0);
    write_port(s_eth_payload_axis_tvalid, 0);
    write_port(s_eth_payload_axis_tuser, 0);        
  }

  void write_header(bit_48& dest_mac,
                    bit_48& src_mac,
                    bit_16& type) {
  stall_on_ready: stall(read_port(s_eth_hdr_ready));

  write_valid: write_port(s_eth_hdr_valid, 1);
  write_src: write_port(s_eth_dest_mac, dest_mac);
  write_dest: write_port(s_eth_src_mac, src_mac);
  write_type: write_port(s_eth_type, type);

  ret: return;

    add_constraint(end(stall_on_ready) < start(write_valid));

    add_constraint(start(write_valid) == start(write_src));
    add_constraint(start(write_valid) == start(write_dest));
    add_constraint(start(write_valid) == start(write_type));

    add_constraint(end(write_valid) + 1 == start(ret));
  }

  void write_byte(bit_8& data,
                  bit_1& last) {

  stall_on_ready: stall(read_port(s_eth_payload_axis_tready));
  write_valid: write_port(s_eth_payload_axis_tvalid, 1);
  write_last: write_port(s_eth_payload_axis_tlast, last);
  write_data: write_port(s_eth_payload_axis_tdata, data);

  ret: return;

    add_constraint(end(stall_on_ready) < start(write_valid));

    add_constraint(start(write_valid) == start(write_last));
    add_constraint(start(write_valid) == start(write_data));

    add_constraint(end(write_valid) + 1 == start(ret));
  }

};

void write_header_func(bit_48& dest_mac,
                       bit_48& src_mac,
                       bit_16& type,
                       eth_axis_tx& transmitter) {
  transmitter.write_header(dest_mac, src_mac, type);
}

void write_byte_func(bit_8& data,
                     bit_1& last,
                     eth_axis_tx& transmitter) {
  transmitter.write_byte(data, last);
}

void write_one_byte_packet(bit_48& dest_mac,
                           bit_48& src_mac,
                           bit_16& type,
                           bit_8& payload,
                           eth_axis_tx& transmitter) {
  transmitter.write_header(dest_mac, src_mac, type);
  transmitter.write_byte(payload, 1);
}

void write_packet(bit_48& dest_mac,
                  bit_48& src_mac,
                  bit_16& type,
                  fifo& payload,
                  bit_32& payload_size,
                  eth_axis_tx& transmitter) {
  transmitter->write_header(dest_mac, src_mac, type);

  bit_32 i;
  for (i = 0; i < payload_size; i = i + 1) {
    bit_1 is_last;
    is_last = i == (payload_size - 1);
    transmitter->write_byte(payload->read_fifo(), is_last);
  }
}
