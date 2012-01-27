--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   11:53:15 01/27/2012
-- Design Name:   
-- Module Name:   /home/aelg/projekt/TSEA27/plogen/Lab1/test.vhd
-- Project Name:  Lab1
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: sender
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.std_logic_unsigned.all;
USE ieee.numeric_std.ALL;
 
ENTITY test IS
END test;
 
ARCHITECTURE behavior OF test IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT sender
    PORT(
         strobe : IN  std_logic;
         d : IN  std_logic_vector(4 downto 1);
         clk : IN  std_logic;
         reset : IN  std_logic;
         u : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal strobe : std_logic := '0';
   signal d : std_logic_vector(4 downto 1) := (others => '0');
   signal clk : std_logic := '0';
   signal reset : std_logic := '0';

 	--Outputs
   signal u : std_logic;

   -- Clock period definitions
   constant clk_period : time := 1us;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: sender PORT MAP (
          strobe => strobe,
          d => d,
          clk => clk,
          reset => reset,
          u => u
        );

   -- Clock process definitions
   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      reset <= '1';
      wait for 100ms;
		reset <= '0';

      wait for clk_period*10;
		
		d <= "0101"
		wait for clk_period;
		strobe <= '1';
		wait for clk_period;
		strobe <= '0';
		wait for clk_period*80;

      d <= "0000";
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		
		d <= d + 3;
		wait for clk_period;
		strobe <= '1';
		wait for clk_period*4;
		strobe <= '0';
		wait for clk_period*80;
		

      wait;
   end process;

END;
