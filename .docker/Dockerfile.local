FROM registry.thinkit.ir/gcc:10 as builder
WORKDIR /src

RUN apt-get update && \
    apt-get install -y --no-install-recommends libev-dev && \
    apt-get clean -y

COPY . .
RUN make

FROM registry.thinkit.ir/debian:buster-slim

RUN apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates libev4 && \
    apt-get clean -y

COPY --from=builder /src/teletin /usr/local/bin/
COPY .docker/docker-entrypoint.sh /

EXPOSE 23/tcp

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["teletin"]
