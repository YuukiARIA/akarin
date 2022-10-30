FROM gcc AS builder

WORKDIR /build
ADD Makefile Makefile
ADD src/ src/
RUN make -j 4 CFLAGS='-Wall -O2 -static'

FROM scratch

COPY --from=builder /build/bin/akarin /usr/local/bin/akarin
ENTRYPOINT ["/usr/local/bin/akarin"]
