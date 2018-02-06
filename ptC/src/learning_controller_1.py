from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.topology.event import EventSwitchEnter, EventSwitchLeave
from ryu.lib.mac import haddr_to_bin
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import ether_types

from topology import load_topology
import networkx as nx

# This function takes as input a networkx graph. It then computes
# the minimum Spanning Tree, and returns it, as a networkx graph.
def compute_spanning_tree(G):

    # The Spanning Tree of G
    ST = nx.minimum_spanning_tree(G)

    return ST

class L2Forwarding(app_manager.RyuApp):
    def __init__(self, *args, **kwargs):
        super(L2Forwarding, self).__init__(*args, **kwargs)

        # Load the topology
        topo_file = 'topology.txt'
        self.G = load_topology(topo_file)

        # For each node in the graph, add an attribute mac-to-port
        for n in self.G.nodes():
            self.G.add_node(n, mactoport={})

        # Compute a Spanning Tree for the graph G
        self.ST = compute_spanning_tree(self.G)

        #print self.G

        print self.get_str_topo(self.G)
        #print self.get_str_topo(self.ST)

    # This method returns a string that describes a graph (nodes and edges, with
    # their attributes). You do not need to modify this method.
    def get_str_topo(self, graph):
        res = 'Nodes\tneighbors:port_id\n'

        att = nx.get_node_attributes(graph, 'ports')
        for n in graph.nodes_iter():
            res += str(n)+'\t'+str(att[n])+'\n'

        res += 'Edges:\tfrom->to\n'
        for f in graph:
            totmp = []
            for t in graph[f]:
                totmp.append(t)
            res += str(f)+' -> '+str(totmp)+'\n'

        return res

    # This method returns a string that describes the Mac-to-Port table of a
    # switch in the graph. You do not need to modify this method.
    def get_str_mactoport(self, graph, dpid):
        res = 'MAC-To-Port table of the switch '+str(dpid)+'\n'

        for mac_addr, outport in graph.node[dpid]['mactoport'].items():
            res += str(mac_addr)+' -> '+str(outport)+'\n'

        return res.rstrip('\n')

    @set_ev_cls(EventSwitchEnter)
    def _ev_switch_enter_handler(self, ev):
        print('enter: %s' % ev)

    @set_ev_cls(EventSwitchLeave)
    def _ev_switch_leave_handler(self, ev):
        print('leave: %s' % ev)

    

    # This method is called every time an OF_PacketIn message is received by 
    # the switch. Here we must calculate the best action to take and install
    # a new entry on the switch's forwarding table if necessary
    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def packet_in_handler(self, ev):
        msg = ev.msg
        dp = msg.datapath
        ofp = dp.ofproto
        ofp_parser = dp.ofproto_parser
	
	pkt = packet.Packet(msg.data)
	eth = pkt.get_protocol(ethernet.ethernet)

	#ADDED
        dst = eth.dst
	src = eth.src
        dpid = dp.id
	
	#ADDED
        #learn mac-to-port mapping so no flood next time
	self.G.node[dpid]['mactoport'][src] = msg.in_port

        #ADDED
	#check if dst in mactoport, if not calc neighbors for flood
	found = 0
	for mac_addr, outport in self.G.node[dpid]['mactoport'].items():
            if dst == mac_addr:
                out_port = outport
                found = 1
                #package actions, send packet
                actions = [ofp_parser.OFPActionOutput(out_port)]
                out = dp.ofproto_parser.OFPPacketOut(datapath=dp,                                 buffer_id=msg.buffer_id, in_port=msg.in_port,                                   actions=actions)
                dp.send_msg(out)
                #if dst was in mactoport, addflow to avoid packet_in in future
                ofproto = dp.ofproto
                match = dp.ofproto_parser.OFPMatch(                                                     in_port=msg.in_port,dl_dst=haddr_to_bin(dst))
                mod = dp.ofproto_parser.OFPFlowMod(                                                     datapath=dp, match=match, cookie=0,                                             command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,                       priority=ofproto.OFP_DEFAULT_PRIORITY,                                          flags=ofproto.OFPFF_SEND_FLOW_REM, actions=actions)
                dp.send_msg(mod)
                
                break
	if found == 0:
          #get neighbors from spanning tree for flood  
          neighbors = [self.ST.node[dpid]['ports']['host']]
          #add non-host neighbors
          for n in self.ST[dpid]:
            neighbors.append(self.ST.node[dpid]['ports'][str(n)])
          #create actions and msg packet
          actions = []
          for out_port in neighbors:
            actions.append(ofp_parser.OFPActionOutput(out_port))
          out = dp.ofproto_parser.OFPPacketOut(datapath=dp,                                 buffer_id=msg.buffer_id, in_port=msg.in_port,                                   actions=actions)
          dp.send_msg(out)





