----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    16:16:22 01/26/2012 
-- Design Name: 
-- Module Name:    sender - Behavioral 
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

entity sender is
    Port ( strobe : in  std_logic;
           d : in  std_logic_vector (4 downto 1);
           clk : in  std_logic;
           reset : in  std_logic;
           u : out  std_logic);
end sender;

architecture Behavioral of sender is
  signal qc: std_logic_vector(4 downto 1);
  signal qr: std_logic_vector(4 downto 0);
  signal qs: std_logic_vector(1 downto 0);
  signal strobe_pulse: std_logic;
  signal cr: std_logic;
begin
 make_strobe_pulse:
 process(clk, reset)
 begin
  if reset = '1' then
   qs <= (others => '0');
	strobe_pulse <= '0';
  elsif rising_edge(clk) then
   case qs is
    when "00" =>
	 if strobe = '1' then
     qs <= "01";
     strobe_pulse <= '1';
	 end if;
    when "01" =>
	 qs <= "10";
    strobe_pulse <= '0';
	 when others =>
	 if strobe = '0' then
     qs <= "00";
	 end if;
   end case;
  end if;
 end process;
 
 counter:
 process(clk, reset)
 begin
  if reset = '1' then
   qc <= (others => '0');
  elsif rising_edge(clk) then
   if strobe_pulse = '1' then
    qc <= (others => '0');
   else qc <= qc + 1;
   end if;
	if qc = "1111" then
	 cr <= '1';
	else
	 cr <= '0';
	 end if;
  end if;
 end process;

 reg:
 process(clk, reset)
 begin
  if reset = '1' then
   qr <= (others => '0');
  elsif rising_edge(clk) then
   if strobe_pulse = '1' then
	 qr <= d & '1';
	elsif cr = '1' then
	 qr(0) <= qr(1);
	 qr(1) <= qr(2);
	 qr(2) <= qr(3);
	 qr(3) <= qr(4);
	 qr(4) <= '0';
	end if;
	u <= qr(0);
  end if;
 end process;

end Behavioral;

