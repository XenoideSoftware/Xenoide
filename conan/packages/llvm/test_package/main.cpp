#include "llvm/Config/llvm-config.h"
#include "llvm/Support/raw_ostream.h"

int main() {
    llvm::outs() << "LLVM version: " << LLVM_VERSION_STRING << "\n";
    return 0;
}
