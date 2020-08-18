# FTGMAC100 tester

Creates bogus network frames to challenge your ARM HW or the QEMU Aspeed model.

Based on an idea of Ziming Zhang <ezrakiez@gmail.com>

## Building

```make KDIR=/path/to/linux/openbmc/```

## Examples

Use the module parameter to tune your frame :

 * tx_desc_entries: Number of TX descriptors (default = 1)
 * bad_address: Bad frame address (default = 0)
 * bad_size: Bad frame size (default = 1) 
 * insert_vlan: Insert VLAN (default true)

Generate a bogus VLAN frame 

```insmod ./ftgmac100-test.ko insert_vlan=1 bad_size=1```
