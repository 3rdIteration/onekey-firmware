# Automatically generated by pb2py
# fmt: off
from .. import protobuf as p

from .BitcoinSignature import BitcoinSignature

if __debug__:
    try:
        from typing import Dict, List  # noqa: F401
        from typing_extensions import Literal  # noqa: F401
    except ImportError:
        pass


class CloseTxSignatureReply(p.MessageType):
    MESSAGE_WIRE_TYPE = 868

    def __init__(
        self,
        signature: BitcoinSignature = None,
    ) -> None:
        self.signature = signature

    @classmethod
    def get_fields(cls) -> Dict:
        return {
            1: ('signature', BitcoinSignature, 0),
        }
