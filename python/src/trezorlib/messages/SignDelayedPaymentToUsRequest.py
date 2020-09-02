# Automatically generated by pb2py
# fmt: off
from .. import protobuf as p

from .ChannelNonce import ChannelNonce
from .NodeId import NodeId
from .Transaction import Transaction

if __debug__:
    try:
        from typing import Dict, List  # noqa: F401
        from typing_extensions import Literal  # noqa: F401
    except ImportError:
        pass


class SignDelayedPaymentToUsRequest(p.MessageType):

    def __init__(
        self,
        node_id: NodeId = None,
        channel_nonce: ChannelNonce = None,
        tx: Transaction = None,
        n: int = None,
    ) -> None:
        self.node_id = node_id
        self.channel_nonce = channel_nonce
        self.tx = tx
        self.n = n

    @classmethod
    def get_fields(cls) -> Dict:
        return {
            1: ('node_id', NodeId, 0),
            2: ('channel_nonce', ChannelNonce, 0),
            3: ('tx', Transaction, 0),
            4: ('n', p.UVarintType, 0),
        }
