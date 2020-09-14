from trezor import wire
from trezor.messages import MessageType

def boot() -> None:
    print("__name__...", __name__)
    wire.add(MessageType.PingRequest, __name__, "Ping")
    wire.add(MessageType.InitRequest, __name__, "Init")
    wire.add(MessageType.GetExtPubKeyRequest, __name__, "GetExtPubKey")
    wire.add(MessageType.NewChannelRequest, __name__, "NewChannel")
    wire.add(MessageType.ReadyChannelRequest, __name__, "ReadyChannel")
    wire.add(MessageType.SignMutualCloseTxRequest, __name__, "SignMutualCloseTx")
    wire.add(MessageType.CheckFutureSecretRequest, __name__, "CheckFutureSecret")
    wire.add(MessageType.GetChannelBasepointsRequest, __name__, "GetChannelBasepoints")
    #wire.add(MessageType.GetPerCommitmentPointRequest, __name__, "GetPerCommitmentPoint")
    # wire.add(MessageType.SignFundingTxRequest, hsmd.__name__, "SignGreeter.SignFundingTx")
    # wire.add(MessageType.SignRemoteCommitmentTxRequest, hsmd.__name__, "SignGreeter.SignRemoteCommitmentTx")
    # wire.add(MessageType.SignCommitmentTxRequest, hsmd.__name__, "SignGreeter.SignCommitmentTx")
    # wire.add(MessageType.SignLocalHTLCTxRequest, hsmd.__name__, "SignGreeter.SignLocalHTLCTx")
    # wire.add(MessageType.SignDelayedPaymentToUsRequest, hsmd.__name__, "SignGreeter.SignDelayedPaymentToUs")
    # wire.add(MessageType.SignRemoteHTLCTxRequest, hsmd.__name__, "SignGreeter.SignRemoteHTLCTx")
    # wire.add(MessageType.SignRemoteHTLCToUsRequest, hsmd.__name__, "SignGreeter.SignRemoteHTLCToUs")
    # wire.add(MessageType.SignPenaltyToUsRequest, hsmd.__name__, "SignGreeter.SignPenaltyToUs")
    # wire.add(MessageType.SignChannelAnnouncementRequest, hsmd.__name__, "SignGreeter.SignChannelAnnouncement")
    # wire.add(MessageType.SignNodeAnnouncementRequest, hsmd.__name__, "SignGreeter.SignNodeAnnouncement")
    # wire.add(MessageType.SignChannelUpdateRequest, hsmd.__name__, "SignGreeter.SignChannelUpdate")
    # wire.add(MessageType.ECDHRequest, hsmd.__name__, "SignGreeter.ECDH")
    # wire.add(MessageType.SignInvoiceRequest, hsmd.__name__, "SignGreeter.SignInvoice")
    # wire.add(MessageType.SignMessageRequest, hsmd.__name__, "SignGreeter.SignMessage")
    # wire.add(MessageType.SignRemoteCommitmentTxPhase2Request, hsmd.__name__, "SignGreeter.SignRemoteCommitmentTxPhase2")
    # wire.add(MessageType.SignLocalCommitmentTxPhase2Request, hsmd.__name__, "SignGreeter.SignLocalCommitmentTxPhase2")
    # wire.add(MessageType.SignMutualCloseTxPhase2Request, hsmd.__name__, "SignGreeter.SignMutualCloseTxPhase2")
