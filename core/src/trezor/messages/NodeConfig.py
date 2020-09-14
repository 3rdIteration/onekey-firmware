# Automatically generated by pb2py
# fmt: off
import protobuf as p

if __debug__:
    try:
        from typing import Dict, List  # noqa: F401
        from typing_extensions import Literal  # noqa: F401
        EnumTypeKeyDerivationStyle = Literal[0, 1, 2]
    except ImportError:
        pass


class NodeConfig(p.MessageType):
    MESSAGE_WIRE_TYPE = 844

    def __init__(
        self,
        key_derivation_style: EnumTypeKeyDerivationStyle = None,
    ) -> None:
        self.key_derivation_style = key_derivation_style

    @classmethod
    def get_fields(cls) -> Dict:
        return {
            1: ('key_derivation_style', p.EnumType("KeyDerivationStyle", (0, 1, 2)), 0),
        }
