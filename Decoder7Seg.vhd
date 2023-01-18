library ieee;
use ieee.std_logic_1164.all;

-- BCD to 7-Segments Decoder (with support for '.' and '-')
entity Decoder7Seg is
    port(
	     data_in  : in  std_logic_vector(3 downto 0);
		  comma    : in  std_logic;
		  data_out : out std_logic_vector(7 downto 0)
    );
end entity Decoder7Seg;

architecture Dataflow of Decoder7Seg is
	signal segments_control : std_logic_vector(6 downto 0);
begin
    segments_control <= -- "0"
	                     "0111111" when data_in="0000" else
								-- "1"
					         "0000110" when data_in="0001" else
								-- "2"
					         "1011011" when data_in="0010" else
								-- "3"
					         "1001111" when data_in="0011" else
								-- "4"
					         "1100110" when data_in="0100" else
								-- "5"
					         "1101101" when data_in="0101" else
								-- "6"
					         "1111101" when data_in="0110" else
								-- "7"
					         "0000111" when data_in="0111" else
								-- "8"
					         "1111111" when data_in="1000" else
								-- "9"
					         "1101111" when data_in="1001" else
					         -- "-"
								"1000000" when data_in="1010" else
								-- "X"
								"1110110" when data_in="1011" else
								-- "Y"
								"1101110" when data_in="1100" else
								-- "Z"
								"0011011" when data_in="1101" else
								-- "[blank]"
								"0000000" when data_in="1110" else
								-- "C" for calibration
					         "0111001";
    
	 data_out <= not(comma) & not(segments_control);
end Dataflow;