#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>

#define NETLINK_USER 31
static struct sock *nl_sk = NULL;

static void my_nl_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from Kernel via Netlink!";
    int res;

    printk(KERN_INFO "my_netlink: Entering %s\n", __FUNCTION__);

    nlh = (struct nlmsghdr *)skb->data;
    
    printk(KERN_INFO "my_netlink: Received msg payload: %s\n", (char *)nlmsg_data(nlh));

    pid = nlh->nlmsg_pid; 

    msg_size = strlen(msg);
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "my_netlink: Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0) {
        printk(KERN_INFO "my_netlink: Error while sending back to user\n");
    }
}

static struct netlink_kernel_cfg cfg = {
    .groups = 1,
    .input = my_nl_recv_msg,
};

static int __init my_netlink_init(void)
{
    printk(KERN_INFO "my_netlink: Loading module...\n");

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "my_netlink: Error creating socket.\n");
        return -10;
    }

    printk(KERN_INFO "my_netlink: Module loaded. Netlink socket created on protocol %d\n", NETLINK_USER);
    return 0;
}

static void __exit my_netlink_exit(void)
{
    printk(KERN_INFO "my_netlink: Unloading module...\n");
    if (nl_sk) {
        netlink_kernel_release(nl_sk);
    }
}

module_init(my_netlink_init);
module_exit(my_netlink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Semchik");
MODULE_DESCRIPTION("Netlink communication module for Kernel 7.0");