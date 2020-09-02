# Automatically generated by pb2py
# fmt: off
from .. import protobuf as p

from .ECDSARecoverableSignature import ECDSARecoverableSignature

if __debug__:
    try:
        from typing import Dict, List  # noqa: F401
        from typing_extensions import Literal  # noqa: F401
    except ImportError:
        pass


class RecoverableNodeSignatureReply(p.MessageType):

    def __init__(
        self,
        signature: ECDSARecoverableSignature = None,
    ) -> None:
        self.signature = signature

    @classmethod
    def get_fields(cls) -> Dict:
        return {
            1: ('signature', ECDSARecoverableSignature, 0),
        }
