library ieee;
use ieee.std_logic_1164.all;

entity Lab3 is
    port(
	     clk, reset               : in    std_logic;
		  i2c_scl, i2c_sda         : inout std_logic;
		  CS_N, ALT_ADRESS         : out   std_logic;
		  switch_axis              : in    std_logic;
		  seg0                     : out   std_logic_vector(7  downto 0);
		  seg1                     : out   std_logic_vector(7  downto 0);
		  seg2                     : out   std_logic_vector(7  downto 0);
		  seg3                     : out   std_logic_vector(7  downto 0);
		  seg4                     : out   std_logic_vector(7  downto 0);
		  seg5                     : out   std_logic_vector(7  downto 0)
	 );
end entity Lab3;

architecture rtl of Lab3 is
	component Lab3_sys is
		port (
			clk_clk                 : in    std_logic                     := 'X'; -- clk
			i2c_external_scl_pad_io : inout std_logic                     := 'X'; -- scl_pad_io
			i2c_external_sda_pad_io : inout std_logic                     := 'X'; -- sda_pad_io
			display_raw_data_export : out   std_logic_vector(29 downto 0);        -- export
			reset_reset_n           : in    std_logic                     := 'X'; -- reset_n
			button_state_export     : in    std_logic
		);
	end component Lab3_sys;
	
	component Decoder7Seg is
		 port(
			  data_in  : in  std_logic_vector(3 downto 0);
			  comma    : in  std_logic;
			  data_out : out std_logic_vector(7 downto 0)
		 );
	end component Decoder7Seg;
	
	signal value_to_display : std_logic_vector(23 downto 0);
	signal comma_to_display : std_logic_vector( 5 downto 0);
begin
   CS_N       <= '1';
	ALT_ADRESS <= '0';

	u_sys : component Lab3_sys
		port map (
			clk_clk                   => clk,      -- clk.clk
			display_raw_data_export(23 downto  0) => value_to_display,
			display_raw_data_export(29 downto 24) => comma_to_display,
			reset_reset_n             => reset,
			i2c_external_scl_pad_io   => i2c_scl,
			i2c_external_sda_pad_io   => i2c_sda,
			button_state_export       => switch_axis
		);
	
	--   +- XXXX|X.|
	u0 : component Decoder7Seg
	   port map(
			data_in  => value_to_display(3 downto 0),
			comma    => comma_to_display(0),
			data_out => seg0
		);

	--   +- XXX|X.|X
	u1 : component Decoder7Seg
	   port map(
			data_in  => value_to_display(7 downto 4),
			comma    => comma_to_display(1),
			data_out => seg1
		);

	--   +- XX|X.|XX
	u2 : component Decoder7Seg
	   port map(
			data_in  => value_to_display(11 downto 8),
			comma    => comma_to_display(2),
			data_out => seg2
		);

	--   +- X|X.|XXX
	u3 : component Decoder7Seg
	   port map(
			data_in  => value_to_display(15 downto 12),
			comma    => comma_to_display(3),
			data_out => seg3
		);

	--   +- |X.|XXXX
	u4 : component Decoder7Seg
	   port map(
			data_in  => value_to_display(19 downto 16),
			comma    => comma_to_display(4),
			data_out => seg4
		);
		
	--  |+-.| XXXXX
	u5 : component Decoder7Seg
	   port map(
			data_in  => value_to_display(23 downto 20),
			comma    => comma_to_display(5),
			data_out => seg5
		);

end rtl;
