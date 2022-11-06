FROM gcc AS builder

WORKDIR /build
ADD Makefile Makefile
ADD src/ src/
RUN make release -j 4 CFLAGS='-Wall -static'

FROM scratch

COPY --from=builder /build/bin/akarin /usr/local/bin/akarin
ENTRYPOINT ["/usr/local/bin/akarin"]
