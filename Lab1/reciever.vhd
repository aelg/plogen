----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    22:01:33 01/29/2012 
-- Design Name: 
-- Module Name:    Reciever - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

---- Uncomment the following library declaration if instantiating
---- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity Reciever is
    Port ( clk : in  STD_LOGIC;
           IR : in  STD_LOGIC;
           reset : in  STD_LOGIC;
           displayout : out  STD_LOGIC_VECTOR (4 downto 1);
			  q: buffer STD_LOGIC_VECTOR (2 downto 1);
			  lamp: out STD_LOGIC;
			  counter4: buffer STD_LOGIC_VECTOR (3 downto 1));
			  
end Reciever;

architecture Behavioral of Reciever is

signal d: std_logic_vector(4 downto 1);
signal reg: std_logic_vector (7 downto 1);
signal counter16: std_logic_vector (4 downto 1);
signal clear16: std_logic;
signal m: std_logic_vector (3 downto 1);

begin

paritetsbitar:
m(1) <= (reg(1) XOR reg(2)) XOR reg(4);
m(2) <= (reg(1) XOR reg(3)) XOR reg(4);
m(3) <= (reg(2) XOR reg(3)) XOR reg(4);

K:
d(1) <= ((m(1) xor reg(5)) and (m(2) xor reg(6)) and (not (m(3) xor reg(7)))) xor reg(1);
d(2) <= ((m(3) xor reg(7)) and (m(1) xor reg(5)) and (not (m(2) xor reg(6)))) xor reg(2);
d(3) <= ((m(2) xor reg(6)) and (m(3) xor reg(7)) and (not (m(1) xor reg(5)))) xor reg(3);
d(4) <= ((m(3) xor reg(7)) and (m(1) xor reg(5)) and (m(2) xor reg(6))) xor reg(4);

lamp <= ((m(1) xor reg(5)) and (m(2) xor reg(6))) or ((m(3) xor reg(7)) and (m(2) xor reg(6))) or ((m(3) xor reg(7)) and (m(1) xor reg(5)));


s_net:
process(clk, reset)
begin
 if reset = '1' then
  q <= (others => '0');
  clear16 <= '0';
  counter4 <= (others => '0');
  reg <= (others => '0');
  displayout<= (others => '0');
 elsif rising_edge(clk) then
  case q is
  when "00" =>
   if IR = '1' then 
	 q <= "01";
	 clear16 <= '1';
	else q <= "00";
	end if;
  when "01" =>
   if counter16 = "1000" then
	 if IR = '1' then 
	  q <= "10";
	  clear16 <= '1';
	  counter4 <= (others => '0');
	 else 
	  q <= "00";
	  clear16 <= '0';
	 end if;
	else clear16 <= '0';
	end if;
  when "10" =>
   clear16 <= '0';
   if counter16 = "1111" then
	 reg(1) <= reg(2);
	 reg(2) <= reg(3);
	 reg(3) <= reg(4);
	 reg(4) <= reg(5);
	 reg(5) <= reg(6);
	 reg(6) <= reg(7);
	 reg(7) <= IR;
	 counter4 <= counter4 + 1;
	elsif counter4 = "111" then
	 displayout <= d;
	 q <= "11";
	end if;
  when others =>
   if IR = '0' then q <= "00";
	end if;
  end case;
 end if;
end process;
 
counter:
process(clk, reset)
begin
 if reset = '1' then
  counter16 <= (others => '0');
 elsif rising_edge(clk) then
  if clear16 = '1' then
   counter16 <= (others => '0');
  else
	counter16 <= counter16 + 1;
  end if;
 end if;
end process;

end Behavioral;

