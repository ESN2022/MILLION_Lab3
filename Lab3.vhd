library ieee;
use ieee.std_logic_1164.all;

entity Lab3 is
    port(
	     clk, reset               : in    std_logic;
		  i2c_scl, i2c_sda         : inout std_logic;
		  -- 
		  CS_N, ALT_ADRESS : out   std_logic;
		  -- temporary
		  pio_7seg                 : out std_logic_vector(7 downto 0)
	 );
end entity Lab3;

architecture rtl of Lab3 is
	component Lab3_sys is
		port (
			clk_clk                  : in    std_logic                    := 'X'; -- clk
			pio_7seg_external_export : out   std_logic_vector(7 downto 0);        -- export
			reset_reset_n            : in    std_logic                    := 'X'; -- reset_n
			i2c_external_scl_pad_io  : inout std_logic                    := 'X'; -- scl_pad_io
			i2c_external_sda_pad_io  : inout std_logic                    := 'X'  -- sda_pad_io
		);
	end component Lab3_sys;
	
begin
   CS_N <= '1';
	ALT_ADRESS <= '0';

	u0 : component Lab3_sys
		port map (
			clk_clk                  => clk,      -- clk.clk
			pio_7seg_external_export => pio_7seg, -- pio_7seg_external.export
			reset_reset_n            => reset,    -- reset.reset_n
			i2c_external_scl_pad_io  => i2c_scl,  -- i2c_external.scl_pad_io
			i2c_external_sda_pad_io  => i2c_sda   -- i2c_external.sda_pad_io
		);
end rtl;
