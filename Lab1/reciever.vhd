----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    13:44:33 01/27/2012 
-- Design Name: 
-- Module Name:    reciever - Behavioral 
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

entity reciever is
    Port ( clk : in  STD_LOGIC;
           IR : in  STD_LOGIC;
			  reset : in STD_LOGIC;
           displayout : out  STD_LOGIC_VECTOR (4 downto 1);
			  q: buffer std_logic_vector(2 downto 1);
			  counter4: buffer std_logic_vector(3 downto 1)
			 );
end reciever;

architecture Behavioral of reciever is

--signal q: std_logic_vector(2 downto 1);
signal reg: std_logic_vector(4 downto 1);
signal counter16: std_logic_vector(4 downto 1);
--signal counter4: std_logic_vector(2 downto 1);
signal clear16: std_logic;

begin

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
	 reg(4) <= IR;
	 counter4 <= counter4 + 1;
	elsif counter4 = "100" then
	 displayout <= reg;
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

