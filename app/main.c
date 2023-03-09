/**		 
 * @Date:               2023.1.9
 * @Revision:           V1.0
 * @author:             jiuvgh
 * @Affiliated unit：   jiuvgh
 * @Description:        基于VScode gcc make opencd环境固件开发工程模板
 * @Email:              18130601770@163.com
 * @github:             https://github.com/IOTJIUVGH/jiuvgh_demo.git
 */
#include <stdio.h>
#include "gpio.h"
#include "peripheral_remap.h"

int main(void) {

	mhal_gpio_open(PA_0, OUTPUT_OPEN_DRAIN_PULL_UP);

	while (1)
	{

	}
}
